#!/usr/bin/env python3

"""
Validate CSV results produced by the C++ EV battery thermal controller.

This automation script performs four jobs:

1. Runs the C++ scenario runner using "make demo".
2. Confirms that the C++ program created a fresh CSV result file.
3. Reads the CSV output from the scenario runner.
4. Compares every scenario result with the expected controller behavior.

This is an educational simulation only.
It does not connect to a real vehicle, battery, ECU, charger, or CAN network.
"""

# The csv module reads comma-separated value files.
import csv

# The subprocess module lets Python run external terminal commands,
# such as "make demo".
import subprocess

# The sys module lets this script return a success code (0) or failure code (1).
import sys

# Path provides a clear, cross-platform way to work with folders and files.
from pathlib import Path


# ---------------------------------------------------------------------------
# Project paths
# ---------------------------------------------------------------------------

# This script is stored here:
# ev-thermal-controller/python/scripts/validate_scenarios.py
#
# SCRIPT_DIRECTORY = python/scripts/
# PYTHON_DIRECTORY = python/
# PROJECT_ROOT     = ev-thermal-controller/
SCRIPT_DIRECTORY = Path(__file__).resolve().parent
PYTHON_DIRECTORY = SCRIPT_DIRECTORY.parent
PROJECT_ROOT = PYTHON_DIRECTORY.parent

# The C++ Makefile and demo executable are located in cpp/.
CPP_DIRECTORY = PROJECT_ROOT / "cpp"

# The C++ scenario runner creates these generated CSV files.
SCENARIO_CSV_PATH = PROJECT_ROOT / "data" / "scenario_results.csv"
PERIODIC_CSV_PATH = PROJECT_ROOT / "data" / "periodic_results.csv"

# ---------------------------------------------------------------------------
# Expected controller results
# ---------------------------------------------------------------------------

# These are the expected outputs for every scenario defined in main.cpp.
#
# Keeping expected values in Python, separate from the C++ controller, is
# important for testing. If C++ behavior changes unexpectedly, this script
# should report a failure rather than automatically accepting the new result.
EXPECTED_RESULTS = {
    "Normal operation": {
        "mode": "NORMAL",
        "cooling_command_percent": 0.0,
        "charging_enabled": "true",
        "charging_derated": "false",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    "Cooling required": {
        "mode": "COOLING",
        "cooling_command_percent": 60.0,
        "charging_enabled": "true",
        "charging_derated": "false",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    "Charging derate": {
        "mode": "DERATE_CHARGING",
        "cooling_command_percent": 100.0,
        "charging_enabled": "true",
        "charging_derated": "true",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    "Invalid temperature sensor": {
        "mode": "FAULT",
        "cooling_command_percent": 100.0,
        "charging_enabled": "false",
        "charging_derated": "false",
        "fault_active": "true",
        "fault_code": "INVALID_TEMPERATURE_SENSOR",
    },
    "CAN message timeout": {
        "mode": "FAULT",
        "cooling_command_percent": 100.0,
        "charging_enabled": "false",
        "charging_derated": "false",
        "fault_active": "true",
        "fault_code": "CAN_MESSAGE_TIMEOUT",
    },
    "Out-of-range temperature": {
        "mode": "FAULT",
        "cooling_command_percent": 100.0,
        "charging_enabled": "false",
        "charging_derated": "false",
        "fault_active": "true",
        "fault_code": "TEMPERATURE_OUT_OF_RANGE",
    },
    "Critical overtemperature": {
        "mode": "FAULT",
        "cooling_command_percent": 100.0,
        "charging_enabled": "false",
        "charging_derated": "false",
        "fault_active": "true",
        "fault_code": "CRITICAL_OVERTEMPERATURE",
    },
}
# These are the expected outputs for every simulated 100 ms controller update
# defined in cpp/src/main.cpp.
#
# This remains separate from the C++ implementation so Python can detect
# unexpected behavior rather than simply trusting generated output.
EXPECTED_PERIODIC_RESULTS = (
    {
        "time_ms": 0,
        "mode": "NORMAL",
        "cooling_command_percent": 0.0,
        "charging_enabled": "true",
        "charging_derated": "false",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    {
        "time_ms": 100,
        "mode": "COOLING",
        "cooling_command_percent": 10.0,
        "charging_enabled": "true",
        "charging_derated": "false",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    {
        "time_ms": 200,
        "mode": "COOLING",
        "cooling_command_percent": 60.0,
        "charging_enabled": "true",
        "charging_derated": "false",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    {
        "time_ms": 300,
        "mode": "DERATE_CHARGING",
        "cooling_command_percent": 100.0,
        "charging_enabled": "true",
        "charging_derated": "true",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    {
        "time_ms": 400,
        "mode": "FAULT",
        "cooling_command_percent": 100.0,
        "charging_enabled": "false",
        "charging_derated": "false",
        "fault_active": "true",
        "fault_code": "CRITICAL_OVERTEMPERATURE",
    },
    {
        "time_ms": 500,
        "mode": "COOLING",
        "cooling_command_percent": 60.0,
        "charging_enabled": "true",
        "charging_derated": "false",
        "fault_active": "false",
        "fault_code": "NONE",
    },
    {
        "time_ms": 600,
        "mode": "NORMAL",
        "cooling_command_percent": 0.0,
        "charging_enabled": "true",
        "charging_derated": "false",
        "fault_active": "false",
        "fault_code": "NONE",
    },
)

# ---------------------------------------------------------------------------
# C++ scenario-runner execution
# ---------------------------------------------------------------------------

def run_cpp_scenario_runner():
    """
    Run the C++ scenario runner.

    The command "make demo" compiles the C++ demo when needed, runs it,
    and creates a fresh data/scenario_results.csv file.
    """

    print("[INFO] Running C++ scenario runner...")

    try:
        # Run "make demo" inside the cpp/ folder.
        #
        # capture_output=True keeps the normal C++ demo output hidden when
        # everything succeeds, so this Python validator stays easy to read.
        # If the C++ program fails, the captured output is printed below.
        result = subprocess.run(
            ["make", "demo"],
            cwd=CPP_DIRECTORY,
            text=True,
            capture_output=True,
        )

    except OSError as error:
        # This catches problems such as Python being unable to start "make".
        print(f"[FAIL] Could not start the C++ scenario runner: {error}")
        return False

    # A return code of 0 means the C++ command completed successfully.
    if result.returncode == 0:
        return True

    # If C++ fails, show the captured output to help debug the problem.
    print("[FAIL] The C++ scenario runner did not complete successfully.")

    if result.stdout:
        print("\nC++ standard output:")
        print(result.stdout)

    if result.stderr:
        print("\nC++ error output:")
        print(result.stderr)

    return False


# ---------------------------------------------------------------------------
# CSV file reading
# ---------------------------------------------------------------------------

def load_csv_results():
    """
    Read the generated CSV file.

    Returns:
        A dictionary where each key is a scenario name and each value is
        the CSV row for that scenario.

    Example:
        results["Cooling required"]["mode"] == "COOLING"
    """

    # Confirm that the C++ program created the expected CSV file.
    if not SCENARIO_CSV_PATH.exists():
        print(f"[FAIL] CSV result file was not created: {CSV_PATH}")
        return None

    # This dictionary will store one CSV row per scenario.
    results_by_scenario = {}

    try:
        # newline="" is the recommended way to open CSV files in Python.
        with SCENARIO_CSV_PATH.open(
            mode="r",
            encoding="utf-8",
            newline=""
        ) as csv_file:
            # DictReader uses the first CSV row as column names.
            #
            # For example, the "mode" column becomes:
            # row["mode"]
            reader = csv.DictReader(csv_file)

            for row in reader:
                scenario_name = row["scenario"]

                # Duplicate scenario names would make validation unclear,
                # so treat them as an error.
                if scenario_name in results_by_scenario:
                    print(
                        "[FAIL] Duplicate scenario found in CSV: "
                        f"{scenario_name}"
                    )
                    return None

                results_by_scenario[scenario_name] = row

    except (OSError, KeyError, csv.Error) as error:
        print(f"[FAIL] Could not read CSV results: {error}")
        return None

    return results_by_scenario


# ---------------------------------------------------------------------------
# Individual scenario validation
# ---------------------------------------------------------------------------

def validate_scenario(
    scenario_name,
    actual,
    expected,
):
    """
    Compare one actual CSV result with its expected controller output.

    Args:
        scenario_name: Readable scenario name, such as "Cooling required".
        actual: Dictionary read from one CSV row.
        expected: Dictionary containing expected output values.

    Returns:
        True if every output field matches; otherwise False.
    """

    # Store all mismatches so one failed scenario can show every problem,
    # instead of stopping after the first incorrect field.
    failures = []

    # These fields are stored as text in the CSV file.
    text_fields = (
        "mode",
        "charging_enabled",
        "charging_derated",
        "fault_active",
        "fault_code",
    )

    # Compare all text-based controller outputs.
    for field_name in text_fields:
        actual_value = actual.get(field_name)
        expected_value = expected[field_name]

        if actual_value != expected_value:
            failures.append(
                f"{field_name}: expected {expected_value}, "
                f"got {actual_value}"
            )

    # Compare cooling-command decimal values with a small tolerance.
    #
    # We avoid comparing decimal values with == because floating-point
    # calculations can sometimes contain tiny rounding differences.
    try:
        actual_cooling = float(actual["cooling_command_percent"])
        expected_cooling = expected["cooling_command_percent"]

        if abs(actual_cooling - expected_cooling) > 0.001:
            failures.append(
                "cooling_command_percent: "
                f"expected {expected_cooling}, got {actual_cooling}"
            )

    except (KeyError, TypeError, ValueError) as error:
        failures.append(
            "cooling_command_percent could not be read: "
            f"{error}"
        )

    # Print a simple PASS result when all output fields match.
    if not failures:
        print(f"[PASS] {scenario_name}")
        return True

    # Print every difference when the scenario fails.
    print(f"[FAIL] {scenario_name}")

    for failure in failures:
        print(f"  - {failure}")

    return False



# Main automation flow

# ---------------------------------------------------------------------------
# Periodic CSV reading and validation
# ---------------------------------------------------------------------------

def load_periodic_csv_results():
    """
    Read the generated periodic CSV file.

    Returns:
        A list of CSV rows in the same order that the C++ simulation wrote
        them. Order matters because each row represents one simulated
        controller update in time.
    """

    # Confirm that the C++ program created the periodic CSV file.
    if not PERIODIC_CSV_PATH.exists():
        print(
            "[FAIL] Periodic CSV result file was not created: "
            f"{PERIODIC_CSV_PATH}"
        )
        return None

    periodic_rows = []

    try:
        # newline="" is the recommended way to open CSV files in Python.
        with PERIODIC_CSV_PATH.open(
            mode="r",
            encoding="utf-8",
            newline=""
        ) as csv_file:
            # DictReader uses the first row as the CSV column names.
            reader = csv.DictReader(csv_file)

            # Preserve CSV row order because it represents simulated time.
            for row in reader:
                periodic_rows.append(row)

    except (OSError, csv.Error) as error:
        print(f"[FAIL] Could not read periodic CSV results: {error}")
        return None

    return periodic_rows


def validate_periodic_step(
    step_number,
    actual,
    expected,
):
    """
    Compare one actual periodic CSV row with expected controller behavior.

    Args:
        step_number: One-based readable step number for terminal output.
        actual: Dictionary read from the periodic CSV row.
        expected: Dictionary containing expected values for this timestamp.

    Returns:
        True if all periodic fields match; otherwise False.
    """

    failures = []

    # Validate the simulated timestamp as an integer number of milliseconds.
    try:
        actual_time_ms = int(actual["time_ms"])

        if actual_time_ms != expected["time_ms"]:
            failures.append(
                "time_ms: "
                f"expected {expected['time_ms']}, got {actual_time_ms}"
            )

    except (KeyError, TypeError, ValueError) as error:
        failures.append(f"time_ms could not be read: {error}")

    # These controller-output fields are stored as text in CSV.
    text_fields = (
        "mode",
        "charging_enabled",
        "charging_derated",
        "fault_active",
        "fault_code",
    )

    # Compare all text-based controller outputs.
    for field_name in text_fields:
        actual_value = actual.get(field_name)
        expected_value = expected[field_name]

        if actual_value != expected_value:
            failures.append(
                f"{field_name}: expected {expected_value}, "
                f"got {actual_value}"
            )

    # Compare cooling-command decimal values with a small tolerance.
    try:
        actual_cooling = float(actual["cooling_command_percent"])
        expected_cooling = expected["cooling_command_percent"]

        if abs(actual_cooling - expected_cooling) > 0.001:
            failures.append(
                "cooling_command_percent: "
                f"expected {expected_cooling}, got {actual_cooling}"
            )

    except (KeyError, TypeError, ValueError) as error:
        failures.append(
            "cooling_command_percent could not be read: "
            f"{error}"
        )

    # Display a readable timestamp when possible.
    timestamp_label = expected["time_ms"]

    # Print a pass line only when every field is correct.
    if not failures:
        print(
            "[PASS] Periodic step "
            f"{step_number} at {timestamp_label} ms"
        )
        return True

    # Print all detected differences for a failed step.
    print(
        "[FAIL] Periodic step "
        f"{step_number} at expected time {timestamp_label} ms"
    )

    for failure in failures:
        print(f"  - {failure}")

    return False


def validate_periodic_intervals(
    periodic_rows,
):
    """
    Confirm that each periodic update occurs exactly 100 ms after the prior
    update.

    Returns:
        True when every interval is 100 ms; otherwise False.
    """

    expected_interval_ms = 100
    previous_time_ms = None
    intervals_valid = True

    for row_number, row in enumerate(periodic_rows, start=1):
        try:
            current_time_ms = int(row["time_ms"])

        except (KeyError, TypeError, ValueError) as error:
            print(
                "[FAIL] Could not read time_ms for periodic row "
                f"{row_number}: {error}"
            )
            intervals_valid = False
            continue

        # The first periodic row has no earlier row to compare.
        if previous_time_ms is not None:
            actual_interval_ms = current_time_ms - previous_time_ms

            if actual_interval_ms != expected_interval_ms:
                print(
                    "[FAIL] Periodic interval after "
                    f"{previous_time_ms} ms: expected "
                    f"{expected_interval_ms} ms, got "
                    f"{actual_interval_ms} ms"
                )
                intervals_valid = False

        previous_time_ms = current_time_ms

    if intervals_valid:
        print(
            "[PASS] All periodic controller updates are "
            "spaced by 100 ms"
        )

    return intervals_valid
def main():
    """
    Run the complete automation workflow.

    1. Run the C++ scenario runner.
    2. Load and validate scenario CSV results.
    3. Load and validate periodic CSV results.
    4. Return 0 for success or 1 for failure.
    """

    # Always create fresh CSV results before validating.
    if not run_cpp_scenario_runner():
        return 1

    # -----------------------------------------------------------------------
    # Validate independent named scenarios
    # -----------------------------------------------------------------------

    print(f"[INFO] Reading scenario CSV results from: {SCENARIO_CSV_PATH}")

    actual_results = load_csv_results()

    # Stop if the scenario CSV file could not be read.
    if actual_results is None:
        return 1

    # Compare expected scenario names with actual scenario names.
    expected_scenarios = set(EXPECTED_RESULTS)
    actual_scenarios = set(actual_results)

    # Identify expected scenarios missing from the generated CSV.
    missing_scenarios = expected_scenarios - actual_scenarios

    for scenario_name in sorted(missing_scenarios):
        print(f"[FAIL] Missing scenario in CSV: {scenario_name}")

    # Identify unexpected scenario names in the generated CSV.
    unexpected_scenarios = actual_scenarios - expected_scenarios

    for scenario_name in sorted(unexpected_scenarios):
        print(f"[FAIL] Unexpected scenario in CSV: {scenario_name}")

    # Count scenarios whose controller output completely matches.
    scenario_passed_count = 0

    # Validate scenarios in the same order as EXPECTED_RESULTS.
    for scenario_name, expected in EXPECTED_RESULTS.items():
        actual = actual_results.get(scenario_name)

        # A missing scenario was already reported above.
        if actual is None:
            continue

        if validate_scenario(
            scenario_name,
            actual,
            expected,
        ):
            scenario_passed_count += 1

    scenario_total_count = len(EXPECTED_RESULTS)

    # Print the scenario-validation summary.
    print()
    print(
        f"Scenario summary: {scenario_passed_count}/"
        f"{scenario_total_count} scenarios passed."
    )

    all_scenarios_present = not missing_scenarios
    no_unexpected_scenarios = not unexpected_scenarios
    all_scenarios_passed = (
        scenario_passed_count == scenario_total_count
    )

    scenarios_valid = (
        all_scenarios_present
        and no_unexpected_scenarios
        and all_scenarios_passed
    )

    # -----------------------------------------------------------------------
    # Validate the simulated 100 ms periodic controller sequence
    # -----------------------------------------------------------------------

    print()
    print(f"[INFO] Reading periodic CSV results from: {PERIODIC_CSV_PATH}")

    periodic_rows = load_periodic_csv_results()

    # Stop if the periodic CSV file could not be read.
    if periodic_rows is None:
        return 1

    expected_periodic_count = len(EXPECTED_PERIODIC_RESULTS)
    actual_periodic_count = len(periodic_rows)

    # A missing or extra row is a validation failure.
    periodic_row_count_valid = (
        actual_periodic_count == expected_periodic_count
    )

    if not periodic_row_count_valid:
        print(
            "[FAIL] Periodic CSV row count: expected "
            f"{expected_periodic_count}, got {actual_periodic_count}"
        )

    # Validate each expected periodic row that is present.
    periodic_passed_count = 0

    for index, expected in enumerate(EXPECTED_PERIODIC_RESULTS):
        # A missing row is already reported by the row-count failure.
        if index >= actual_periodic_count:
            continue

        actual = periodic_rows[index]

        if validate_periodic_step(
            index + 1,
            actual,
            expected,
        ):
            periodic_passed_count += 1

    # Confirm each consecutive simulation sample is 100 ms apart.
    periodic_intervals_valid = validate_periodic_intervals(periodic_rows)

    # Print the periodic-validation summary.
    print()
    print(
        f"Periodic summary: {periodic_passed_count}/"
        f"{expected_periodic_count} periodic steps passed."
    )

    all_periodic_steps_passed = (
        periodic_passed_count == expected_periodic_count
    )

    periodic_valid = (
        periodic_row_count_valid
        and periodic_intervals_valid
        and all_periodic_steps_passed
    )

    # -----------------------------------------------------------------------
    # Overall pass/fail result
    # -----------------------------------------------------------------------

    print()

    if scenarios_valid and periodic_valid:
        print("[PASS] Complete validation passed.")
        return 0

    print("[FAIL] Complete validation failed.")
    return 1


# This ensures main() runs only when this file is executed directly.
if __name__ == "__main__":
    sys.exit(main())

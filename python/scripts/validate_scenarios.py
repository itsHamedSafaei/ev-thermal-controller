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

# The C++ scenario runner creates this generated CSV file.
CSV_PATH = PROJECT_ROOT / "data" / "scenario_results.csv"


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
    if not CSV_PATH.exists():
        print(f"[FAIL] CSV result file was not created: {CSV_PATH}")
        return None

    # This dictionary will store one CSV row per scenario.
    results_by_scenario = {}

    try:
        # newline="" is the recommended way to open CSV files in Python.
        with CSV_PATH.open(
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


def main():
    """
    Run the complete automation workflow.

    1. Run the C++ scenario runner.
    2. Load generated CSV results.
    3. Check missing/unexpected scenarios.
    4. Validate every expected controller result.
    5. Return 0 for success or 1 for failure.
    """

    # Always create fresh results before validating.
    if not run_cpp_scenario_runner():
        return 1

    print(f"[INFO] Reading CSV results from: {CSV_PATH}")

    actual_results = load_csv_results()

    # Stop if the CSV file could not be read.
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

    # Count the scenarios whose controller output completely matches.
    passed_count = 0

    # Validate scenarios in the same order they appear in EXPECTED_RESULTS.
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
            passed_count += 1

    total_count = len(EXPECTED_RESULTS)

    # Print the final automation summary.
    print()
    print(
        f"Summary: {passed_count}/{total_count} "
        "scenarios passed."
    )

    # The script succeeds only when:
    # - All expected scenarios appeared in the CSV.
    # - No unexpected scenarios appeared.
    # - Every scenario output matched expectations.
    all_scenarios_present = not missing_scenarios
    no_unexpected_scenarios = not unexpected_scenarios
    all_scenarios_passed = passed_count == total_count

    if (
        all_scenarios_present
        and no_unexpected_scenarios
        and all_scenarios_passed
    ):
        return 0

    return 1


# This ensures main() runs only when this file is executed directly.
if __name__ == "__main__":
    sys.exit(main())
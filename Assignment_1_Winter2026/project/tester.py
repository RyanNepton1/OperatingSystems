import pathlib 
import subprocess 
import difflib
import shutil
import os

BASE_DIR = pathlib.Path(__file__).parent.resolve()
test_directory = BASE_DIR / "test-cases"
MYSH = BASE_DIR / "src" / "mysh"
TEMP_DIR = BASE_DIR / "test-temp"  # Temporary directory for each test

def run_test(test_file : pathlib.Path): 
    expected_results = test_file.with_name(test_file.stem + "_result.txt") 
    print(f"Running test: {expected_results.name}") 
    if not expected_results.exists(): 
        print(f"[SKIP] {test_file.name} (missing results file)") 
        return
    
    # Create a fresh temporary directory for this test
    if TEMP_DIR.exists():
        shutil.rmtree(TEMP_DIR)
    TEMP_DIR.mkdir()
    
    # Save current directory and change to temp directory
    original_dir = os.getcwd()
    os.chdir(TEMP_DIR)
    
    try: 
        result = subprocess.run( 
            [MYSH], 
            input=test_file.read_text(), 
            text=True, 
            capture_output=True, 
            check=False) 
    except Exception as e: 
        print(f"[ERROR] {test_file.name} (execution failed: {e})") 
        os.chdir(original_dir)
        return
    finally:
        # Always return to original directory
        os.chdir(original_dir)
    
    actual_output = result.stdout 
    expected_output = expected_results.read_text() 
    if actual_output == expected_output: 
        print(f"[PASS] {test_file.name}") 
    else: 
        print(f"[FAIL] {test_file.name}") 
        diff = difflib.unified_diff( 
            expected_output.splitlines(), 
            actual_output.splitlines(), 
            fromfile="expected", 
            tofile="actual", 
            lineterm="") 
        for line in diff: 
            print(line) 
        print()
    
    # Clean up temp directory after test
    if TEMP_DIR.exists():
        shutil.rmtree(TEMP_DIR)

def main(): 
    for test_file in sorted(test_directory.iterdir()): 
        if test_file.is_file() and "_" not in test_file.name: 
            run_test(test_file) 

main()
import subprocess
import os
import ctypes

def run_as_admin(exe_path):
    try:
        # ShellExecuteEx equivalent to "runas"
        ctypes.windll.shell32.ShellExecuteW(None, "runas", exe_path, None, None, 1)
    except Exception as e:
        print(f"Failed to run {exe_path} as admin: {e}")

def run_normal(exe_path):
    try:
        subprocess.Popen(exe_path, shell=True)
    except Exception as e:
        print(f"Failed to run {exe_path}: {e}")

if __name__ == "__main__":
    # Set correct relative or absolute paths
    run_as_admin(os.path.abspath("picoPulse/pulseControl/pulseControl.exe"))
    run_as_admin(os.path.abspath("2 JoyToKey/( JoyToKey - RUN AS ADMIN ).exe"))
    run_normal(os.path.abspath("3 GtunerIV/Gtuner.exe"))

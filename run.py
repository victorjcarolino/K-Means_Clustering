import os
import subprocess
import matplotlib.pyplot as plt

def run_executables(dataset_filename):
    executables_dir = "bin/"
    runtimes = []

    for filename in os.listdir(executables_dir):
        if os.path.isfile(os.path.join(executables_dir, filename)):
            executable_path = os.path.join(executables_dir, filename)
            command = [executable_path, dataset_filename]
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            _, error = process.communicate()

            if error:
                print(f"Error running {filename}: {error.decode()}")
            else:
                runtime = float(process.stdout.decode().strip())
                runtimes.append(runtime)

    return runtimes

def plot_runtimes(runtimes):
    if not runtimes:
        print("No runtimes to plot.")
        return

    min_runtime = min(runtimes)
    avg_runtime = sum(runtimes) / len(runtimes)
    max_runtime = max(runtimes)

    plt.bar(["Minimum", "Average", "Maximum"], [min_runtime, avg_runtime, max_runtime])
    plt.xlabel("Runtime")
    plt.ylabel("Time (seconds)")
    plt.title("Runtime Statistics")
    plt.show()

if __name__ == "__main__":
    dataset_filename = input("Enter the dataset filename: ")
    runtimes = run_executables(dataset_filename)
    plot_runtimes(runtimes)
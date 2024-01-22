import subprocess
import time
import matplotlib.pyplot as plt
import csv

def run_experiment(K, N_values, file_size, c_program):
    time_results = []

    for N in N_values:
        start_time = time.time()

        subprocess.run([c_program, str(K), str(N), str(file_size)])

        end_time = time.time()
        elapsed_time = end_time - start_time
        time_results.append(elapsed_time)

    return time_results

def write_to_csv(N_values, time_results, filename):
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['N Values', 'Elapsed Time (seconds)'])
        for N, time_result in zip(N_values, time_results):
            writer.writerow([N, time_result])

    print(f"Results written to '{filename}'")

def main():
    K = 1000  # K değeri
    N_values = [1, 2, 3]  # Farklı N değerleri
    file_size = 1000000  # Dosya boyutu (integer sayı adedi)
    c_programs = ['./findtopk', './findtopk_mqueue', './findtopk_thread']  # C programlarının adları

    for i, c_program in enumerate(c_programs, start=1):
        time_results = run_experiment(K, N_values, file_size, c_program)

        # CSV dosyasına ve grafik oluştur
        filename = f'experiment_results_{i}.csv'
        write_to_csv(N_values, time_results, filename)

        plt.plot(N_values, time_results, marker='o', label=f'C Program {i}')
    
    plt.xlabel('N Values')
    plt.ylabel('Elapsed Time (seconds)')
    plt.title('Elapsed Time for Different N Values')
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    main()


import tkinter as tk
from tkinter import messagebox, filedialog
from tkcalendar import Calendar
import time
from datetime import datetime
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import serial

arduino = serial.Serial(port='COM6', baudrate=9600, timeout=1)

def arduino_set_time(date_time_str):
    try:
        time.sleep(2)
        arduino.write(date_time_str.encode())
        print(f"Wysłano do Arduino: {date_time_str}")
        messagebox.showinfo("Sukces", f"Data i godzina {date_time_str} wysłana do Arduino")
    except serial.SerialException as e:
        print(f"Błąd komunikacji z Arduino: {e}")
        messagebox.showerror("Błąd", f"Błąd komunikacji z Arduino: {e}")

def send_time():
    selected_date = calendar.get_date()
    selected_time = time_entry.get()
    
    try:
        time.strptime(selected_time, "%H:%M:%S")
    except ValueError:
        messagebox.showerror("Błąd", "Niepoprawny format godziny. Użyj HH:MM.")
        return
    
    date_obj = datetime.strptime(selected_date, "%Y-%m-%d")
    weekday_num = date_obj.weekday()
    weekday_name = str(weekday_num)

    formatted_date = date_obj.strftime("%y%m%d")
    formatted_time = selected_time.replace(":", "")
    date_time_str = f"{formatted_date}{weekday_name}{formatted_time}x"
    print(date_time_str)
    arduino_set_time(date_time_str)

def send_current_time():
    current_time = time.strftime("%Y-%m-%d %H:%M:%S")
    current_date_obj = datetime.strptime(current_time, "%Y-%m-%d %H:%M:%S")
    weekday_num = current_date_obj.weekday()
    weekday_name = str(weekday_num)
    
    formatted_date = current_date_obj.strftime("%y%m%d")
    formatted_time = time.strftime("%H%M%S")
    date_time_str = f"{formatted_date}{weekday_name}{formatted_time}x"
    print(date_time_str)
    arduino_set_time(date_time_str)

def process_data(data):
    times = []
    tempos = []
    start_time = data[0].strip()
    start_time = start_time[:6] + start_time[7:14]
    try:
        start_time = datetime.strptime(start_time, "%y%m%d%H%M%S")
    except ValueError as e:
        messagebox.showerror("Błąd", f"Błąd w danych: {e}")
        return
    
    for i in range(1, len(data)):
        prev = data[i - 1].strip()
        curr = data[i].strip()
        
        prev = prev[:6] + prev[7:14]
        curr = curr[:6] + curr[7:14]
        
        try:
            prev_date = datetime.strptime(prev, "%y%m%d%H%M%S")
            curr_date = datetime.strptime(curr, "%y%m%d%H%M%S")
        except ValueError as e:
            messagebox.showerror("Błąd", f"Błąd w danych: {e}")
            return
        
        time_diff = (curr_date - start_time).total_seconds()
        times.append(time_diff)
        
        if (curr_date - prev_date).total_seconds() == 0:
            tempo = 0
        else:
            tempo = 25 / (curr_date - prev_date).total_seconds()
        tempos.append(tempo)
    
    return times, tempos


def open_file():
    file_path = filedialog.askopenfilename(filetypes=[("Text Files", "*.TXT")], initialdir="D:/")
    if file_path:
        try:
            with open(file_path, 'r') as file:
                data = file.readlines()
                data = data[0:-2]
                print(data)
                times, tempos = process_data(data)
                plot_graph(times, tempos)
        except Exception as e:
            messagebox.showerror("Błąd", f"Nie udało się otworzyć pliku: {e}")

def plot_graph(times, tempos):
    try:
        fig, ax = plt.subplots(figsize=(5, 4))
        ax.plot(times, tempos, marker='o')
        ax.set_title("Prędkość na 25 metrach")
        ax.set_xlabel("Czas (sekundy)")
        ax.set_ylabel("Prędkość (m/s)")
        canvas = FigureCanvasTkAgg(fig, master=right_frame)
        canvas.get_tk_widget().pack(padx=20, pady=20)
        canvas.draw()
    except ValueError:
        messagebox.showerror("Błąd", "Błędne dane w pliku. Upewnij się, że zawiera liczby.")


root = tk.Tk()
root.title("Przycisk pływaka")

main_frame = tk.Frame(root)
main_frame.pack(fill=tk.BOTH, expand=True)

left_frame = tk.Frame(main_frame)
left_frame.pack(side=tk.LEFT, fill=tk.Y, padx=10, pady=10)

calendar = Calendar(left_frame, selectmode='day', date_pattern='yyyy-mm-dd')
calendar.pack(pady=20)

time_label = tk.Label(left_frame, text="Wpisz godzinę (HH:MM:SS):")
time_label.pack(pady=5)
time_entry = tk.Entry(left_frame)
time_entry.pack(pady=5)

send_button = tk.Button(left_frame, text="Wyślij datę i godzinę do Arduino", command=send_time)
send_button.pack(pady=20)

send_current_time_button = tk.Button(left_frame, text="Wyślij aktualną datę i godzinę do Arduino", command=send_current_time)
send_current_time_button.pack(pady=20)
right_frame = tk.Frame(main_frame)
right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, padx=10, pady=10)
open_file_button = tk.Button(right_frame, text="Otwórz plik tekstowy", command=open_file)
open_file_button.pack(pady=10)


def on_close():
    root.quit()  
    root.destroy() 

root.protocol("WM_DELETE_WINDOW", on_close)


root.mainloop()
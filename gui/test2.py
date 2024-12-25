import os
import socket
import sys
import threading
import time
import tkinter as tk
from tkinter import ttk
import configparser
import keyboard
import subprocess

if getattr(sys, "frozen", False):
    program_directory = os.path.dirname(os.path.abspath(sys.executable))
        
else:
    program_directory = os.getcwd()

SETTINGS_FILE = program_directory + "/settings.ini"
# 알파벳과 숫자 매핑
ALPHABET = [chr(i) for i in range(97, 123)] + [chr(i) for i in range(65, 91)]  # a-z + A-Z
ALPHABET_MAP = {letter: idx + 1 for idx, letter in enumerate(ALPHABET)}  # a=1, ..., z=26, A=27, ..., Z=52
REVERSE_ALPHABET_MAP = {v: k for k, v in ALPHABET_MAP.items()}  # 1=a, ..., 52=Z

# 스킬 이름과 영어 매핑
SKILLS = {
    "순환": "cycle",
    "혼파술": "divorce",
    "금강불체": "diamond",
    "퇴마주": "exorcism"
}

# 이미 선택된 알파벳 추적
used_letters = set()
# 눌린 키를 추적하기 위한 집합
# 소켓 변수
client_socket = None

is_cycle_start = False
is_diamond_start = False

# 설정 저장
def save_settings(skill_key, selected_letter):
    """
    INI 파일에 설정 저장
    """
    print(f"save_settings called: skill_key={skill_key}, selected_letter={selected_letter}")
    config = configparser.ConfigParser()
    if os.path.exists(SETTINGS_FILE):
        config.read(SETTINGS_FILE)
    if "Settings" not in config:
        config["Settings"] = {}

    skill_name = SKILLS[skill_key]
    config["Settings"][skill_name] = str(ALPHABET_MAP[selected_letter])  # 숫자로 저장
    with open(SETTINGS_FILE, "w") as configfile:
        config.write(configfile)
    print(f"Settings saved: {skill_key} ({skill_name}) -> {selected_letter} (Value: {ALPHABET_MAP[selected_letter]})")

# 설정 로드
def load_settings():
    """
    INI 파일에서 설정 로드
    """
    config = configparser.ConfigParser()
    loaded_settings = {}
    if os.path.exists(SETTINGS_FILE):
        config.read(SETTINGS_FILE)
        for skill_key, skill_name in SKILLS.items():
            if skill_name in config["Settings"]:
                cycle_value = config.getint("Settings", skill_name, fallback=1)
                loaded_settings[skill_key] = REVERSE_ALPHABET_MAP.get(cycle_value, "a")
    # 누락된 설정은 기본값으로 초기화
    for skill_key in SKILLS.keys():
        if skill_key not in loaded_settings:
            loaded_settings[skill_key] = "a"
    return loaded_settings

# 사용 가능한 알파벳 업데이트
def update_available_letters(dropdowns):
    """
    드롭박스의 가능한 값 갱신
    """
    global used_letters
    used_letters.clear()

    # 현재 사용 중인 값 추적
    for skill_key, (var, _) in dropdowns.items():
        current_value = var.get()
        if current_value in ALPHABET:
            used_letters.add(current_value)

    # 각 드롭박스 업데이트
    for skill_key, (var, dropdown) in dropdowns.items():
        current_value = var.get()
        available_letters = [letter for letter in ALPHABET if letter not in used_letters or letter == current_value]
        dropdown["values"] = tuple(available_letters)

# 드롭박스 값 변경 이벤트
def on_letter_change(event, skill_name, var):
    """
    드롭박스 값 변경 시 호출
    """
    print(f"on_letter_change called: skill_name={skill_name}, selected_value={var.get()}")
    save_settings(skill_name, var.get())  # 설정 저장
    update_available_letters(dropdowns)  # 다른 드롭박스 값 갱신
    english_skill_name = SKILLS[skill_name]  # 영어 이름으로 변환
    skill_value = ALPHABET_MAP[var.get()]  # 숫자로 변환
    send_message_to_dll(f"{english_skill_name}:{skill_value}")  # DLL로 숫자 전송

# DLL과의 연결
def start_server():
    global client_socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind(("127.0.0.1", 9698))
    server_socket.listen(1)
    print("Server listening on 127.0.0.1:9698")

    while True:
        print("Waiting for DLL to connect...")
        try:
            client_socket, addr = server_socket.accept()
            print(f"Connection from {addr} established.")
            send_initial_settings()  # 연결되었을 때 설정 값 전송
            handle_dll_connection(client_socket)
        except Exception as e:
            update_connect_status("대기중")
            print(f"Error while accepting connection: {e}")

def send_initial_settings():
    """
    DLL 연결 시 설정 파일의 현재 값을 전송
    """
    update_connect_status("준비 완료")
    if client_socket:
        for skill_key, skill_name in SKILLS.items():  # key, value 쌍으로 처리
            try:
                # 설정 값을 숫자로 변환
                value = ALPHABET_MAP[load_settings()[skill_key]]  # 숫자로 변환
                # 영어 이름과 숫자 값으로 메시지 전송
                send_message_to_dll(f"{skill_name}:{value}")
                print(f"Sent to DLL: {skill_name}:{value}")
                time.sleep(0.4)
            except KeyError:
                print(f"KeyError: Missing value for {skill_key} in loaded_settings")
            except Exception as e:
                print(f"Error while sending initial settings for {skill_key}: {e}")

        

# DLL 메시지 처리
def handle_dll_connection(socket):
    global client_socket
    while True:
        try:
            data = socket.recv(1024)
            if not data:
                print("DLL disconnected.")
                update_connect_status("대기중")
                break
            print(f"Received from DLL: {data.decode()}")
        except Exception as e:
            update_connect_status("대기중")
            print(f"Error: {e}")
            break
    client_socket = None
    socket.close()

# DLL로 메시지 전송
def send_message_to_dll(message):
    global client_socket
    if client_socket:
        try:
            client_socket.sendall(message.encode())
            print(f"Sent to DLL: {message}")
        except Exception as e:
            print(f"Error sending to DLL: {e}")
    else:
        update_connect_status("대기중")
        print("DLL not connected.")

def handle_cycle():
    global is_cycle_start
    if is_cycle_start:
        send_message_to_dll("stop cycle")
        is_cycle_start = False
    else:
        send_message_to_dll("start cycle")
        is_cycle_start = True
    print("Ctrl + 1 detected")

def handle_diamond():
    global is_diamond_start
    if is_diamond_start:
        send_message_to_dll("stop diamond")
        is_diamond_start = False
    else:
        send_message_to_dll("start diamond")
        is_diamond_start = True
    print("Ctrl + 2 detected")

def send_message_to_dll(message):
    print(f"Sent to DLL: {message}")

def start_hotkey_listener():
    # 핫키를 등록
    keyboard.add_hotkey('ctrl+1', handle_cycle)
    keyboard.add_hotkey('ctrl+2', handle_diamond)

    # 이벤트가 발생할 때까지 대기
    keyboard.wait()

# Tkinter GUI 생성
def main():
    # 설정 로드
    global loaded_settings
    loaded_settings = load_settings()

    # 소켓 서버 스레드 시작
    threading.Thread(target=start_server, daemon=True).start()
    # 핫키 리스너 시작
    threading.Thread(target=start_hotkey_listener, daemon=True).start()

    # Tkinter GUI 시작
    root = tk.Tk()
    root.title("Skill Configuration")
    root.geometry("300x250")

    global dropdowns
    dropdowns = {}

    for skill_key in SKILLS.keys():
        frame = tk.Frame(root)
        frame.pack(pady=5, fill="x")

        # 레이블
        label = tk.Label(frame, text=skill_key, width=10, anchor="w")
        label.pack(side="left", padx=5)

        # 드롭박스
        letter_var = tk.StringVar(value=loaded_settings.get(skill_key, "a"))
        dropdown = ttk.Combobox(frame, textvariable=letter_var, state="readonly")
        dropdown.pack(side="left", padx=5, fill="x", expand=True)
        dropdowns[skill_key] = (letter_var, dropdown)

        # 이벤트 바인딩
        dropdown.bind("<<ComboboxSelected>>", lambda event, skill_name=skill_key, var=letter_var: on_letter_change(event, skill_name, var))

    # 초기화
    update_available_letters(dropdowns)
    # 설명 문구 추가
    description_frame = tk.Frame(root)
    description_frame.pack(side="bottom", pady=20)

    label_cycle = tk.Label(description_frame, text="순환 핫키 : 컨트롤 + 1", font=("Arial", 12))
    label_cycle.pack()

    label_diamond = tk.Label(description_frame, text="금강 불체 : 컨트롤 + 2", font=("Arial", 12))
    label_diamond.pack()

    global label_connect
    label_connect = tk.Label(description_frame, text="대기중", font=("Arial", 12))
    label_connect.pack()
    exe_path = "subprocess.exe"  # 실제 경로로 변경
    process = subprocess.Popen(exe_path, shell=True)

    root.mainloop()

def update_connect_status(status):
    """
    연결 상태 레이블 텍스트를 업데이트
    """
    global label_connect
    if label_connect:
        label_connect.config(text=status)

if __name__ == "__main__":
    main()

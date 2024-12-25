import socket
import threading
import tkinter as tk
from tkinter import ttk
import configparser
import os

# INI 파일 경로
SETTINGS_FILE = "settings.ini"

# 알파벳과 숫자 매핑
ALPHABET = [chr(i) for i in range(97, 123)] + [chr(i) for i in range(65, 91)]  # a-z + A-Z
ALPHABET_MAP = {letter: idx + 1 for idx, letter in enumerate(ALPHABET)}  # a=1, ..., z=26, A=27, ..., Z=52
REVERSE_ALPHABET_MAP = {v: k for k, v in ALPHABET_MAP.items()}  # 1=a, ..., 52=Z

# 스킬 이름과 영어 매핑
SKILLS = {
    "순환": "cycle",
    "파혼술": "divorce",
    "금강불체": "diamond",
    "퇴마주": "exorcism"
}

# 이미 선택된 알파벳 추적
used_letters = set()

# 소켓 변수
client_socket = None

# 설정 저장
def save_settings(skill_key, selected_letter):
    """
    INI 파일에 설정 저장
    """
    config = configparser.ConfigParser()
    if os.path.exists(SETTINGS_FILE):
        config.read(SETTINGS_FILE)
    if "Settings" not in config:
        config["Settings"] = {}

    # 선택한 값을 영어 이름으로 저장
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
                cycle_value = config.getint("Settings", skill_name, fallback=1)  # 기본값: 1 (a)
                loaded_settings[skill_key] = REVERSE_ALPHABET_MAP.get(cycle_value, "a")  # 숫자를 알파벳으로 변환
    # 기본값 설정
    for skill_key in SKILLS.keys():
        if skill_key not in loaded_settings:
            loaded_settings[skill_key] = "a"  # 기본값
    return loaded_settings

# 사용 가능한 알파벳 업데이트
def update_available_letters(dropdowns):
    """
    사용 가능한 알파벳을 드롭박스별로 업데이트
    """
    global used_letters
    used_letters = {var.get() for _, (var, _) in dropdowns.values()}  # 현재 선택된 값 추적

    for skill_key, (var, dropdown) in dropdowns.items():
        current_value = var.get()
        # 현재 선택된 값을 포함하여 사용 가능한 알파벳 목록 생성
        available_letters = [letter for letter in ALPHABET if letter not in used_letters or letter == current_value]
        dropdown["values"] = available_letters  # 드롭박스 값 갱신

# DLL과의 연결 대기 및 메시지 처리
def start_server():
    global client_socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # 포트 재사용 옵션
    server_socket.bind(("127.0.0.1", 9698))
    server_socket.listen(1)
    print("Python server listening on 127.0.0.1:9698")

    while True:
        print("Waiting for DLL to connect...")
        try:
            client_socket, addr = server_socket.accept()
            print(f"Connection from {addr} established.")
            handle_dll_connection(client_socket)
        except Exception as e:
            print(f"Error while accepting connection: {e}")

# DLL 메시지 처리
def handle_dll_connection(socket):
    global client_socket
    while True:
        try:
            data = socket.recv(1024)
            if not data:
                print("DLL disconnected.")
                break

            # DLL에서 수신한 메시지 출력
            message = data.decode()
            print(f"Received from DLL: {message}")

        except Exception as e:
            print(f"Error: {e}")
            break

    client_socket = None  # 연결 해제
    socket.close()

# DLL에 메시지 전송
def send_message_to_dll(message):
    global client_socket
    if client_socket:
        try:
            client_socket.sendall(message.encode())
            print(f"Sent to DLL: {message}")
        except Exception as e:
            print(f"Error sending to DLL: {e}")
    else:
        print("DLL is not connected. Waiting for reconnection.")

# Tkinter GUI 생성
def main():
    # 설정 로드
    loaded_settings = load_settings()

    # 소켓 서버 스레드 시작
    threading.Thread(target=start_server, daemon=True).start()

    # Tkinter GUI 시작
    root = tk.Tk()
    root.title("Skill Configuration")
    root.geometry("400x400")

    # 각 스킬에 대해 드롭박스 생성
    dropdowns = {}
    global used_letters

    for skill_key in SKILLS.keys():
        frame = tk.Frame(root)
        frame.pack(pady=5, fill="x")

        # 스킬 이름 레이블
        skill_label = tk.Label(frame, text=skill_key, width=10, anchor="w")
        skill_label.pack(side="left", padx=5)

        # 드롭박스
        letter_var = tk.StringVar(value=loaded_settings.get(skill_key, "a"))  # 초기값 설정
        used_letters.add(letter_var.get())
        dropdown = ttk.Combobox(frame, textvariable=letter_var, state="readonly")
        dropdowns[skill_key] = (letter_var, dropdown)
        dropdown.pack(side="left", padx=5, fill="x", expand=True)

        # 이벤트 바인딩
        dropdown.bind("<<ComboboxSelected>>", lambda event, skill_name=skill_key, var=letter_var: on_letter_change(event, skill_name, var))

    # 초기화
    update_available_letters(dropdowns)

    # DLL 호출 버튼
    def call_test():
        send_message_to_dll("cycle")

    button = tk.Button(root, text="Call DLL Function", command=call_test)
    button.pack(pady=20)

    root.mainloop()

if __name__ == "__main__":
    main()

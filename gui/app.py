import ctypes
import time
import threading
from datetime import datetime
import psutil
from queue import Queue
from collections import defaultdict

# Windows API 상수
PROCESS_ALL_ACCESS = 0x1F0FFF

# Windows API 함수 정의
OpenProcess = ctypes.windll.kernel32.OpenProcess
ReadProcessMemory = ctypes.windll.kernel32.ReadProcessMemory
CloseHandle = ctypes.windll.kernel32.CloseHandle

def get_process_by_name(process_name):
    """프로세스 이름으로 PID 찾기"""
    for proc in psutil.process_iter(['pid', 'name']):
        if proc.info['name'].lower() == process_name.lower():
            return proc.info['pid']
    return None

def read_memory(process_handle, address, size):
    """특정 주소에서 메모리 읽기"""
    buffer = ctypes.create_string_buffer(size)
    bytesRead = ctypes.c_size_t()
    if ReadProcessMemory(process_handle, ctypes.c_void_p(address), buffer, size, ctypes.byref(bytesRead)):
        return buffer.raw
    return None

def read_by_address(process_handle, address, size=64, encoding="utf-16le"):
    """지정된 주소에서 메모리를 읽고 내용을 반환"""
    data = read_memory(process_handle, address, size)
    if data:
        return data.decode(encoding, errors='ignore')
    return None

def memory_reader(process_handle, address, size, encoding, data_queue):
    """메모리 데이터를 지속적으로 읽어서 큐에 저장"""
    last_chat = ""
    while True:
        result = read_by_address(process_handle, address, size, encoding)
        if result:
            # 첫 번째 NUL 이전 텍스트 추출
            cleaned_result = result.split("\x00", 1)[0]
            if cleaned_result != last_chat:
                last_chat = cleaned_result
                data_queue.put(cleaned_result)  # 큐에 추가

def find_pattern_in_memory(process_handle, pattern, start_address=0x0, scan_size=0x100000):
    """
    메모리에서 지정된 바이트 패턴을 찾아 그 위치의 주소를 반환.
    - process_handle: 프로세스 핸들
    - pattern: 바이트 패턴 (리스트 또는 바이트 객체로 입력)
    - start_address: 검색을 시작할 메모리 주소
    - scan_size: 검색할 메모리 영역의 크기
    """
    pattern_len = len(pattern)
    for offset in range(0, scan_size - pattern_len):
        # 메모리 읽기
        memory = read_memory(process_handle, start_address + offset, pattern_len)
        if memory and match_pattern(memory, pattern):
            return start_address + offset  # 일치하는 주소 반환
    return None  # 패턴을 찾지 못한 경우

def match_pattern(memory, pattern):
    """메모리에서 패턴을 매칭하는 함수 (와일드카드 처리)"""
    for i in range(len(pattern)):
        if pattern[i] != 0xFF and pattern[i] != memory[i]:
            return False
    return True

def file_writer(data_queue):
    """큐에서 데이터를 가져와 파일에 기록하며, 캐릭터별 킬/데스를 누적 계산"""
    kill_count = defaultdict(int)  # 캐릭터별 킬수를 저장
    death_count = defaultdict(int)  # 캐릭터별 데스수를 저장
    special_events = []  # 특수 이벤트 메시지 리스트

    with open("result.txt", "a", encoding="utf-8") as file:
        while True:
            result = data_queue.get()  # 큐에서 데이터를 가져옴
            if result:
                file.write(result + "\n")  # result.txt에 기록
                file.flush()  # 즉시 디스크에 기록

                # 데이터에서 킬 및 데스 정보 추출
                if "[정보]" in result and "님이" in result and "무찔렀습니다!" in result:
                    try:
                        # 데이터 형식: "[정보] AA님이 BB 무찔렀습니다!"
                        parts = result.split("님이")
                        killer = parts[0].strip().split(" ")[-1]  # 킬 한 사람 ('AA')
                        victim = parts[1].strip().split(" ")[0]  # 킬 당한 사람 ('BB')

                        # 킬수 및 데스수 업데이트
                        if killer != "님":  # 잘못된 데이터 무시
                            kill_count[killer] += 1
                        if victim != "님":  # 잘못된 데이터 무시
                            death_count[victim] += 1

                        # 콘솔 출력: 전체 캐릭터별 킬/데스
                        print("전체 캐릭터 킬/데스:")
                        for character in set(kill_count.keys()).union(death_count.keys()):
                            kills = kill_count.get(character, 0)
                            deaths = death_count.get(character, 0)
                            print(f"{character}: {kills}킬 / {deaths}데스")
                        print("-" * 30)  # 구분선

                        # 정렬된 전체 기록을 kill_death.txt에 덮어쓰기
                        with open("kill_death.txt", "w", encoding="utf-8") as kd_file:
                            # 킬수를 기준으로 내림차순 정렬
                            sorted_records = sorted(
                                set(kill_count.keys()).union(death_count.keys()),
                                key=lambda char: kill_count.get(char, 0),
                                reverse=True
                            )
                            for character in sorted_records:
                                kills = kill_count.get(character, 0)
                                deaths = death_count.get(character, 0)
                                kd_file.write(f"{character}: {kills}킬 / {deaths}데스\n")
                    except IndexError:
                        print("킬 정보를 파싱할 수 없습니다:", result)
                
                # 특정 이벤트 메시지 감지
                elif result.startswith("<<") and result.endswith(">>"):
                    special_events.append(result)

                    # 특수 이벤트를 덮어쓰기 방식으로 기록
                    with open("special_events.txt", "w", encoding="utf-8") as events_file:
                        events_file.write("\n".join(special_events) + "\n")
                else:
                    print(result)  # 콘솔 출력

def monitor_process(process_name, pattern, start_address=0x0, scan_size=0x100000, encoding="utf-16le"):
    """효율적인 메모리 모니터링 (스레드 기반)"""
    pid = get_process_by_name(process_name)
    if not pid:
        print(f"프로세스 {process_name}를 찾을 수 없습니다.")
        return

    # 프로세스 핸들 열기
    process_handle = OpenProcess(PROCESS_ALL_ACCESS, False, pid)
    if not process_handle:
        print("프로세스를 열 수 없습니다.")
        return

    try:
        # 패턴 검색하여 주소 찾기
        address = find_pattern_in_memory(process_handle, pattern, start_address, scan_size)
        if address is None:
            print("패턴을 찾을 수 없습니다.")
            return

        print({hex(address)})

        # 데이터 큐 생성
        data_queue = Queue()

        # 스레드 생성
        reader_thread = threading.Thread(target=memory_reader, args=(process_handle, address, 64, encoding, data_queue))
        writer_thread = threading.Thread(target=file_writer, args=(data_queue,))

        # 데몬 스레드 설정
        reader_thread.daemon = True
        writer_thread.daemon = True

        # 스레드 시작
        reader_thread.start()
        writer_thread.start()

        # 메인 스레드 유지
        while True:
            time.sleep(1)
    finally:
        CloseHandle(process_handle)

def test(data_queue):
    """result.txt 파일의 내용을 한 줄씩 읽어 큐에 넣는 테스트 함수"""
    try:
        with open("test.txt", "r", encoding="utf-8") as file:
            for line in file:
                cleaned_line = line.strip()
                if cleaned_line:  # 빈 줄은 제외
                    data_queue.put(cleaned_line)
                # time.sleep(0.5)  # 테스트를 위해 읽기 간격 조정
    except FileNotFoundError:
        print("result.txt 파일을 찾을 수 없습니다.")
    finally:
        print("테스트 함수 실행 완료.")

def test_start():
    data_queue = Queue()

    # 스레드 생성
    reader_thread = threading.Thread(target=test, args=(data_queue,))
    writer_thread = threading.Thread(target=file_writer, args=(data_queue,))

    # 데몬 스레드 설정
    reader_thread.daemon = True
    writer_thread.daemon = True

    # 스레드 시작
    reader_thread.start()
    writer_thread.start()
    while True:
        time.sleep(1)

# 사용 예시
if __name__ == "__main__":
    # 원하는 바이트 패턴 (0x5B 00 ?? ?? ?? ?? 5D 00 20 00 3A 00 20 00)
    pattern = bytes([0x5B, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x5D, 0x00, 0x20, 0x00, 0x3A, 0x00, 0x20, 0x00])

    monitor_process(
        process_name="winbaram.exe",  # 대상 프로세스 이름
        pattern=pattern,               # 바이트 패턴
        start_address=0x00100000,      # 검색 시작 주소 (예시)
        scan_size=0x100000,            # 검색할 메모리 크기 (예시)
        encoding="utf-16le"            # 문자열 인코딩
    )

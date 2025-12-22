import os

def search_string(root_path, target):
    for root, dirs, files in os.walk(root_path):
        for filename in files:
            filepath = os.path.join(root, filename)
            try:
                with open(filepath, 'r', errors='ignore') as f:
                    for lineno, line in enumerate(f, start=1):
                        if target in line:
                            print(f"[FOUND] {filepath}:{lineno} -> {line.strip()}")
            except:
                pass  # Bỏ qua file không đọc được

if __name__ == "__main__":
    folder_path = "."  # thư mục đang đứng
    search_term = "esp_bt_controller_deinit"

    search_string(folder_path, search_term)
import os
import subprocess
import urllib.request
import zipfile
import json
import shutil
import locale
import sys
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
import ctypes
import logging

# Configuration de la journalisation
log_file = "mod_unpacker.log"
logging.basicConfig(filename=log_file, level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

# Vérifiez si le fichier de log peut être créé
try:
    open(log_file, 'a').close()
except Exception as e:
    print(f"Error creating log file: {e}")
    sys.exit(1)

# Fonction pour installer pip si nécessaire
def ensure_pip():
    try:
        __import__('pip')
    except ImportError:
        print("Pip not found, installing...")
        subprocess.check_call([sys.executable, '-m', 'ensurepip', '--default-pip'])

# Fonction pour installer les packages manquants
def install_packages():
    ensure_pip()
    required_packages = ["customtkinter", "colorama"]
    for package in required_packages:
        try:
            __import__(package)
        except ImportError:
            subprocess.check_call([sys.executable, "-m", "pip", "install", package])

# Installation des packages manquants
install_packages()

# Initialisation des packages installés
import customtkinter as ctk
from colorama import init, Fore, Style

# Masquer la console CMD
def hide_console():
    if os.name == 'nt':
        ctypes.windll.user32.ShowWindow(ctypes.windll.kernel32.GetConsoleWindow(), 0)

hide_console()

# Initialisation de colorama
init(autoreset=True)

# Détection de la langue du système
lang = locale.getlocale()[0][:2]

# Charger les traductions
with open('translations.json', 'r', encoding='utf-8') as f:
    translations = json.load(f)

# Choisir les textes basés sur la langue du système
if lang in translations:
    t = translations[lang]
else:
    t = translations['en']  # Par défaut en anglais

# URLs et chemins
REPAK_URL = "https://github.com/trumank/repak/releases/latest"
REPAK_ZIP = "repak.zip"
REPAK_DIR = os.path.join(os.getcwd(), "repak")
AES_KEY = "0x6F80948821CA338739A24D4D9F778BCAC0996B2EF2A73897A789C68AFF05174E"

def download_latest_repak():
    latest_url = "https://github.com/trumank/repak/releases/latest"
    with urllib.request.urlopen(latest_url) as response:
        html = response.read().decode('utf-8')
        zip_index = html.find('repak_cli-x86_64-pc-windows-msvc.zip')
        start_index = html.rfind('https://github.com/', 0, zip_index)
        end_index = zip_index + len('repak_cli-x86_64-pc-windows-msvc.zip')
        download_url = html[start_index:end_index]
        return download_url

def download_and_extract(url, zip_name, extract_to):
    if 'repak' in url:
        url = download_latest_repak()
    urllib.request.urlretrieve(url, zip_name)
    with zipfile.ZipFile(zip_name, 'r') as zip_ref:
        zip_ref.extractall(extract_to)
    os.remove(zip_name)

def setup_repak():
    if not os.path.exists(REPAK_DIR):
        try:
            download_and_extract(REPAK_URL, REPAK_ZIP, REPAK_DIR)
        except Exception as e:
            log_message(f"Error setting up Repak: {e}")
            logging.error(f"Error setting up Repak: {e}")

def log_message(message):
    console_text.config(state=tk.NORMAL)
    console_text.insert(tk.END, message + '\n')
    console_text.config(state=tk.DISABLED)
    console_text.see(tk.END)
    logging.info(message)

def unpack_pak_repak(pak_file):
    repak_executable = os.path.join(REPAK_DIR, "repak.exe")
    if not os.path.exists(repak_executable):
        error_message = t['repak_not_configured']
        messagebox.showerror("Erreur", error_message)
        log_message(error_message)
        logging.error(error_message)
        return
    command = f'"{repak_executable}" --aes-key {AES_KEY} unpack "{pak_file}"'
    log_message(t['unpacking_repak'])
    try:
        subprocess.run(command, shell=True, check=True)
        success_message = f"Unpacked {pak_file}"
        log_message(success_message)
        log_message("Unpacking completed.")
    except subprocess.CalledProcessError as e:
        error_message = f"Error unpacking: {e}"
        log_message(error_message)
        logging.error(error_message)

def repack_pak(folder_name, output_pak):
    repak_executable = os.path.join(REPAK_DIR, "repak.exe")
    if not os.path.exists(repak_executable):
        error_message = t['repak_not_configured']
        messagebox.showerror("Erreur", error_message)
        log_message(error_message)
        logging.error(error_message)
        return
    command = f'"{repak_executable}" pack -v --version V12 "{folder_name}" -- "{output_pak}"'
    log_message(t['packing_repak'])
    try:
        subprocess.run(command, shell=True, check=True)
        success_message = f"Packed {output_pak}"
        log_message(success_message)
        log_message("Packing completed.")
    except subprocess.CalledProcessError as e:
        error_message = f"Error packing: {e}"
        log_message(error_message)
        logging.error(error_message)

def choose_file():
    file_path = filedialog.askopenfilename(initialdir=os.getcwd())
    return file_path

def choose_folder():
    folder_path = filedialog.askdirectory(initialdir=os.getcwd())
    return folder_path

def setup():
    setup_repak()

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Mod Unpacker")
        self.geometry("500x400")  # Agrandir la fenêtre pour inclure la console
        self.resizable(False, False)
        self.iconbitmap("512x512bb.ico")

        self.label = ctk.CTkLabel(self, text=t['welcome'])
        self.label.pack(pady=10)

        self.unpack_repak_button = ctk.CTkButton(self, text=t['option_1'], command=self.unpack_repak)
        self.unpack_repak_button.pack(pady=10)

        self.repack_button = ctk.CTkButton(self, text=t['option_3'], command=self.repack)
        self.repack_button.pack(pady=10)

        global console_text
        console_frame = ctk.CTkFrame(self)
        console_frame.pack(fill=tk.BOTH, expand=True, pady=(10, 0))

        console_text = scrolledtext.ScrolledText(console_frame, height=10, state=tk.DISABLED, bg='black', fg='white')
        console_text.pack(fill=tk.BOTH, expand=True)

    def unpack_repak(self):
        pak_file = choose_file()
        if pak_file:
            console_text.pack(fill=tk.BOTH, expand=True)
            unpack_pak_repak(pak_file)

    def repack(self):
        folder_name = choose_folder()
        output_pak = filedialog.asksaveasfilename(defaultextension=".pak", filetypes=[("PAK files", "*.pak")])
        if folder_name and output_pak:
            console_text.pack(fill=tk.BOTH, expand=True)
            repack_pak(folder_name, output_pak)

if __name__ == "__main__":
    try:
        setup()
        app = App()
        app.mainloop()
    except Exception as e:
        logging.error(f"Unhandled exception: {e}")
        raise

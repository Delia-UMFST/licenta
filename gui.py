import os
import subprocess
import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext, ttk
from tkinterdnd2 import DND_FILES, TkinterDnD


class PluginRunnerApp:
    def __init__(self, root):
        self.root = root
        root.title("G++ Plugin Runner")
        root.geometry("1200x700")
        root.minsize(800, 500)

        root.rowconfigure(0, weight=1)
        root.columnconfigure(0, weight=1)

        main_frame = tk.Frame(root, padx=10, pady=10)
        main_frame.grid(row=0, column=0, sticky="nsew")
        main_frame.rowconfigure(3, weight=1)
        main_frame.columnconfigure(0, weight=1)

        control_frame = tk.Frame(main_frame)
        control_frame.grid(row=0, column=0, sticky="ew", pady=5)
        
        label = tk.Label(control_frame, text="Drag a .c file here or click 'Browse' to select one:")
        label.pack(side="left", padx=5)

        browse_button = tk.Button(control_frame, text="Browse .c File", command=self.select_file)
        browse_button.pack(side="left", padx=5)

        self.drop_area = tk.Label(main_frame, text="Drop File Here", bg="lightgray", height=3)
        self.drop_area.grid(row=1, column=0, sticky="ew", pady=5)
        self.drop_area.drop_target_register(DND_FILES)
        self.drop_area.dnd_bind('<<Drop>>', self.drop_file)


        self.paned_window = ttk.PanedWindow(main_frame, orient=tk.HORIZONTAL)
        self.paned_window.grid(row=3, column=0, sticky="nsew", pady=10)

        self.file_content_box = scrolledtext.ScrolledText(
            self.paned_window,
            wrap=tk.WORD,
            font=("Consolas", 12),
            state='disabled'
        )
        self.paned_window.add(self.file_content_box, weight=1)

        self.output_box = scrolledtext.ScrolledText(
            self.paned_window,
            wrap=tk.WORD,
            font=("Consolas", 12),
            state='normal'
        )
        self.paned_window.add(self.output_box, weight=1)

        main_frame.columnconfigure(0, weight=1)
        main_frame.rowconfigure(3, weight=1)

    def display_file_contents(self, file_path):
        self.file_content_box.config(state='normal')
        self.file_content_box.delete('1.0', tk.END)       
        try:
            with open(file_path, 'r') as file:
                content = file.read()
                self.file_content_box.insert(tk.END, content)
                self.file_content_box.config(state='disabled')
        except Exception as e:
            self.file_content_box.insert(tk.END, f"Error reading file: {str(e)}")
            self.file_content_box.config(state='disabled')

    def run_plugins(self, file_path):
        self.output_box.config(state='normal')
        self.output_box.delete('1.0', tk.END)

        if not file_path.endswith('.c'):
            self.output_box.insert(tk.END, "Please select a .c file.\n")
            self.output_box.config(state='disabled')
            return

        if not os.path.exists(file_path):
            self.output_box.insert(tk.END, "File does not exist.\n")
            self.output_box.config(state='disabled')
            return

        plugin1_path = os.path.join(os.path.dirname(__file__), "plugin1.so")
        if not os.path.exists(plugin1_path):
            self.output_box.insert(tk.END, f"Plugin not found at {plugin1_path}\n")
            self.output_box.config(state='disabled')
            return

        plugin2_path = os.path.join(os.path.dirname(__file__), "plugin2.so")
        if not os.path.exists(plugin2_path):
            self.output_box.insert(tk.END, f"Plugin not found at {plugin2_path}\n")
            self.output_box.config(state='disabled')
            return
        
        plugin3_path = os.path.join(os.path.dirname(__file__), "plugin3.so")
        if not os.path.exists(plugin3_path):
            self.output_box.insert(tk.END, f"Plugin not found at {plugin3_path}\n")
            self.output_box.config(state='disabled')
            return

        plugin4_path = os.path.join(os.path.dirname(__file__), "plugin4.so")
        if not os.path.exists(plugin4_path):
            self.output_box.insert(tk.END, f"Plugin not found at {plugin4_path}\n")
            self.output_box.config(state='disabled')
            return
        
        plugin5_path = os.path.join(os.path.dirname(__file__), "plugin5.so")
        if not os.path.exists(plugin5_path):
            self.output_box.insert(tk.END, f"Plugin not found at {plugin5_path}\n")
            self.output_box.config(state='disabled')

        self.display_file_contents(file_path)

        command = [
            "g++-13",
            "-O0",
            f"-fplugin={os.path.abspath(plugin1_path)}",
            f"-fplugin={os.path.abspath(plugin2_path)}",
            f"-fplugin={os.path.abspath(plugin3_path)}",
            f"-fplugin={os.path.abspath(plugin4_path)}",
            f"-fplugin={os.path.abspath(plugin5_path)}",
            file_path
        ]

        try:
            result = subprocess.run(command, capture_output=True, text=True)
            if result.stdout:
                self.output_box.insert(tk.END, f"Other output:\n{result.stdout}\n")
            if result.stderr:
                self.output_box.insert(tk.END, f"Warnings:\n{result.stderr}\n")
        except Exception as e:
            self.output_box.insert(tk.END, f"Exception:\n{str(e)}\n")
        
        self.output_box.config(state='disabled')

    def drop_file(self, event):
        file_path = event.data.strip('{}')
        self.run_plugins(file_path)

    def select_file(self):
        file_path = filedialog.askopenfilename(filetypes=[("C files", "*.c")])
        if file_path:
            self.run_plugins(file_path)


if __name__ == "__main__":
    root = TkinterDnD.Tk()
    app = PluginRunnerApp(root)
    root.mainloop()
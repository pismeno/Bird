from pathlib import Path
import shutil

def copy_file(file_path: Path, dst_path: Path) -> None:
    shutil.copy(file_path, dst_path)
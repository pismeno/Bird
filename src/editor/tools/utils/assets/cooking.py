from pathlib import Path
from typing import BinaryIO
import shutil

def stream_file(file_path: Path, out_file: BinaryIO) -> None:
    with open(file_path, "rb") as in_file:
        shutil.copyfileobj(in_file, out_file)
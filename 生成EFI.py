import subprocess
import shutil
import os
from pathlib import Path

# 基础路径配置
BASE_DIR = Path(__file__).parent.resolve()

# 源文件路径
PYTHON_SCRIPT = BASE_DIR / "python/生成图片.py"
IMAGE_DIR = BASE_DIR / "python/Image"

# 目标路径
EFI_TARGET_DIR = BASE_DIR / "EFI/TianJI"
IMAGE_TARGET = EFI_TARGET_DIR / "Image"

def run_python_script():
    try:
        print(f"正在执行脚本: {PYTHON_SCRIPT}")
        subprocess.run(['python', str(PYTHON_SCRIPT)], check=True)
        print("图片生成完成")
    except subprocess.CalledProcessError as e:
        print(f"脚本执行失败: {e}")
        raise

def move_image_dir():
    try:
        EFI_TARGET_DIR.mkdir(parents=True, exist_ok=True)
        print(f"正在移动文件夹到: {IMAGE_TARGET}")
        if IMAGE_TARGET.exists():
            shutil.rmtree(str(IMAGE_TARGET))
        shutil.move(str(IMAGE_DIR), str(IMAGE_TARGET))
        print("Image文件夹移动完成")
    except Exception as e:
        print(f"文件夹移动失败: {e}")
        raise

if __name__ == "__main__":
    try:
        run_python_script()
        move_image_dir()
        print("所有操作执行完毕")
    except Exception as e:
        print(f"执行过程中发生错误: {e}")
        exit(1)
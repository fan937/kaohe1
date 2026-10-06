import json
import sys

try:
    import yaml
    HAS_YAML = True
except ImportError:
    HAS_YAML = False

from color import Color

def load_config(filename: str):
    try:
        with open(filename, "r", encoding="utf-8") as f:
            if filename.endswith((".yaml", ".yml")):
                if not HAS_YAML:
                    print(f"{Color.RED}Error: 需要pyyaml，请执行 pip install pyyaml{Color.RESET}")
                    sys.exit(1)
                return yaml.safe_load(f)
            elif filename.endswith(".json"):
                return json.load(f)
            else:
                print(f"{Color.RED}Error: 仅支持 .yaml / .yml / .json{Color.RESET}")
                sys.exit(1)
    except FileNotFoundError:
        print(f"{Color.RED}Error: 配置文件 '{filename}' 不存在{Color.RESET}")
        sys.exit(1)
    except (yaml.YAMLError, json.JSONDecodeError):
        print(f"{Color.RED}Error: 配置文件语法错误{Color.RESET}")
        sys.exit(1)
    except Exception as e:
        print(f"{Color.RED}Error: 读取配置失败：{e}{Color.RESET}")
        sys.exit(1)
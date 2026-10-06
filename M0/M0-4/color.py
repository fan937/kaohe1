import sys

def is_tty():
    return hasattr(sys.stdout, "isatty") and sys.stdout.isatty()


class Color:
    if is_tty():
        GREEN  = '\033[92m'
        RED    = '\033[91m'
        YELLOW = '\033[93m'
        BLUE   = '\033[94m'
        RESET  = '\033[0m'
    else:
        GREEN = RED = YELLOW = BLUE = RESET = ''
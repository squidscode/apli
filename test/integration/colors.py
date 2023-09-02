import os

no_color = os.environ.get('NO_COLOR')

def disable_on_no_color(fn):
    if no_color:
        return print
    else:
        return fn

@disable_on_no_color
def prRed(skk, **kwargs): print("\033[91m {}\033[00m" .format(skk), **kwargs)
@disable_on_no_color
def prGreen(skk, **kwargs): print("\033[92m {}\033[00m" .format(skk), **kwargs)
@disable_on_no_color
def prYellow(skk, **kwargs): print("\033[93m {}\033[00m" .format(skk), **kwargs)
@disable_on_no_color
def prLightPurple(skk, **kwargs): print("\033[94m {}\033[00m" .format(skk), **kwargs)
@disable_on_no_color
def prPurple(skk, **kwargs): print("\033[95m {}\033[00m" .format(skk), **kwargs)
@disable_on_no_color
def prCyan(skk, **kwargs): print("\033[96m {}\033[00m" .format(skk), **kwargs)
@disable_on_no_color
def prLightGray(skk, **kwargs): print("\033[97m {}\033[00m" .format(skk), **kwargs)
@disable_on_no_color
def prBlack(skk, **kwargs): print("\033[98m {}\033[00m" .format(skk), **kwargs)
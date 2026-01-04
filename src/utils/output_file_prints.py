def print_output(fh, *args, **kwargs):
    print(*args, **kwargs)
    print(*args, **kwargs, file=fh)

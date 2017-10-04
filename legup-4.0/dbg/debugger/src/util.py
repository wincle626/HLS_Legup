def roundupIntToNearest(x, interval):
    if x % interval == 0:
        return x
    else:
        return x + interval - x % interval
import numpy as np

def main():
    size = input("Enter the exponent size: ")
    size = int(size)
    arr = np.random.randint(low=0, high=2**31, size=size, dtype=np.int32)
    filename = f"array_{size}.bin"
    arr.tofile(filename)
    print(f"Saved {filename} ({arr.nbytes / (1024**3):.2f} GiB)")
    pass

if __name__ == "__main__":
    main()
    pass
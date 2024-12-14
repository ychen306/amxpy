import ctypes

lib = ctypes.CDLL("./amx.so")


class GlobalState(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_byte * 512),
        ("y", ctypes.c_byte * 512),
        ("z", ctypes.c_byte * 4096),
        ("mem", ctypes.c_byte * (256 + 128)),
    ]

    def __repr__(self):
        return f"x = {bytes(self.x)}\ny = {bytes(self.y)}\nz = {bytes(self.z)}\nmem = {bytes(self.mem)}"


funcs = [
    "EXTRX",
    "EXTRY",
    "FMA16",
    "FMA32",
    "FMA64",
    "FMS16",
    "FMS32",
    "FMS64",
    "GENLUT",
    "LDX",
    "LDY",
    "LDZ",
    "LDZI",
    "MAC16",
    "MATFP",
    "MATINT",
    "STX",
    "STY",
    "STZ",
    "STZI",
    "VECFP",
]

for name in funcs:
    f = getattr(lib, name)
    f.argtypes = [ctypes.c_uint64, ctypes.POINTER(GlobalState)]
    f.restype = GlobalState


if __name__ == "__main__":
    state = GlobalState()
    for i, c in zip(range(len(state.mem)), b"hello, world\n"):
        state.mem[i] = c
    state.x[0] = ord("@")
    new_state = lib.LDX(1 << 56, state)
    print(bytes(new_state.x))

import os


def bytes_to_c_arr(data, lowercase=True):
    return [format(b, "#04x" if lowercase else "#04X") for b in data]


def file_to_array(file, var_name):
    with open(file, "rb") as f:
        res_str = "const char " + var_name + "[] = {"
        bytes_arr = bytes_to_c_arr(f.read())
        if bytes_arr[-1] != "0x00":
            bytes_arr.append("0x00")
        for i, b in enumerate(bytes_arr):
            if i % 10 == 0:
                res_str += "\n        "
            res_str += b + ", "
        res_str += "\n};"
    return res_str


src_folder = os.path.join("..", "src")

with open(os.path.join(src_folder, "shaders.h"), "w") as f:
    f.write(
        file_to_array(
            os.path.join(src_folder, "shaders", "main.frag"),
            "src_shaders_main_frag",
        )
    )
    f.write("\n\n")
    f.write(
        file_to_array(
            os.path.join(src_folder, "shaders", "main.vert"),
            "src_shaders_main_vert",
        )
    )

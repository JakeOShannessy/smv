#!/bin/python
import subprocess
import os
import re


def run_fds(directory, filename, processes=None):
    print("running", filename)
    args = ["fds6.9.1", filename]
    if processes:
        args.append("-p")
        args.append(processes)
    subprocess.run(args, shell=False,
                   capture_output=False, text=True, cwd=directory)
    print("completed", filename)


def run_smv_script(directory, filename, smv_path="smokeview"):
    print("running", filename)
    args = [smv_path, "-runscript", filename]
    result = subprocess.run(args, shell=False,
                            capture_output=True, text=True, cwd=directory)
    if result.returncode != 0:
        print(result.stdout)
        print(result.stderr)
        raise Exception(f'smokeview failed: case: {filename}')
    print("completed", filename)


cmp_regex = re.compile('[^\\(]*\\((.*)\\)')


def compare_images(image1, image2, out_path) -> float:
    print("comparing", image1, image2)
    blur1 = "blur1.png"
    blur2 = "blur2.png"
    intermediate_path = "inter1.png"
    blur_image(image1, blur1)
    blur_image(image2, blur2)
    args = ["compare", "-metric", "rmse", blur1, blur2, intermediate_path]
    # args = ["/home/jake/smv/.vscode/build/smokeview","-runscript", filename]
    # if processes:
    #     args.append("-p")
    #     args.append(processes)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    output = subprocess.run(args, shell=False,
                            capture_output=True, text=True, cwd=".")
    diff = None
    m = cmp_regex.match(output.stderr)
    if m:
        diff = float(m.group(1))
    print("comparison", output.stdout)
    inter3 = "out1.png"
    composite_image(blur1, blur2, inter3, diff)
    inter5a = "inter5a.png"
    annotate_image(image1, inter5a, "Base")
    inter5b = "inter5b.png"
    annotate_image(image2, inter5b, "Current")
    subprocess.run(["magick", "montage", inter5a, inter5b, inter3, "-geometry", "+3+1", out_path], shell=False,
                   capture_output=False, text=True, cwd=".")
    print("output", out_path)
    return diff


def annotate_image(input, output, label):
    print("labelling", input)
    args = ["magick", input, "-gravity", "Center", "-pointsize",
            "24", f"label:{label}", "-append"]
    args.append(output)
    print("label args:", args)
    if os.path.dirname(output):
        os.makedirs(os.path.dirname(output), exist_ok=True)
    subprocess.run(args, shell=False,
                   capture_output=False, text=True, cwd=".")


def blur_image(input, output):
    print("blurring", input)
    args = ["magick", input, "-blur", "0x2"]
    args.append(output)
    print("blur args:", args)
    if os.path.dirname(output):
        os.makedirs(os.path.dirname(output), exist_ok=True)
    subprocess.run(args, shell=False,
                   capture_output=False, text=True, cwd=".")


def composite_image(input1, input2, output, diff):
    print("composing", input1, input2)
    inter2 = "inter2.png"
    args = ["composite", input1, input2, "-compose", "difference", inter2]
    if os.path.dirname(output):
        os.makedirs(os.path.dirname(output), exist_ok=True)
    subprocess.run(args, shell=False,
                   capture_output=False, text=True, cwd=".")
    args2 = ["magick", inter2, "-channel", "RGB", "-negate"]
    if diff != None:
        args2 += ["-gravity", "Center", "-pointsize",
                  "24", f"label:rmse: {diff}", "-append"]
    args2.append(output)
    print("args2", args2)
    subprocess.run(args2, shell=False,
                   capture_output=False, text=True, cwd=".")

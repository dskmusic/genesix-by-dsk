#!/usr/bin/env python3
"""Package build artefacts exactly as the GitHub Release does. Output: dist/"""
import argparse, hashlib, pathlib, shutil, tarfile, zipfile

NAME = "Genesix by DSK"
SLUG = "Genesix-by-DSK"

def zip_tree(src: pathlib.Path, out: pathlib.Path):
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        if src.is_file():
            z.write(src, src.name)
        else:
            for f in sorted(src.rglob("*")):
                z.write(f, pathlib.Path(src.name) / f.relative_to(src))

def tar_tree(src: pathlib.Path, out: pathlib.Path):
    with tarfile.open(out, "w:gz") as t:
        t.add(src, arcname=src.name)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--platform", choices=["win", "linux"], required=True)
    ap.add_argument("--build", default="build")
    ap.add_argument("--config", default="Release")
    ap.add_argument("--out", default="dist")
    ap.add_argument("--checksums-only", action="store_true")
    a = ap.parse_args()
    art = pathlib.Path(a.build) / "Genesix_artefacts" / a.config
    out = pathlib.Path(a.out); out.mkdir(exist_ok=True)
    vst3 = art / "VST3" / f"{NAME}.vst3"
    made = []
    if a.checksums_only:
        pass
    elif a.platform == "win":
        exe = art / "Standalone" / f"{NAME}.exe"
        made.append(out / f"{SLUG}-win-x64-vst3.zip"); zip_tree(vst3, made[-1])
        made.append(out / f"{SLUG}-win-x64-standalone.zip"); zip_tree(exe, made[-1])
    else:
        exe = art / "Standalone" / NAME
        made.append(out / f"{SLUG}-linux-x64-vst3.tar.gz"); tar_tree(vst3, made[-1])
        made.append(out / f"{SLUG}-linux-x64-standalone.tar.gz"); tar_tree(exe, made[-1])
    with open(out / f"SHA256SUMS-{a.platform}.txt", "w", newline="\n") as s:
        for f in sorted(out.iterdir()):
            if f.suffix in (".zip", ".gz", ".exe"):
                s.write(f"{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.name}\n")
    for f in made: print("packaged", f)

if __name__ == "__main__":
    main()

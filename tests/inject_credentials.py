#!/usr/bin/env python3
"""Insert AppsFlyer test credentials into a local bundle after the native build.

The resulting APK must be zipaligned and signed; an iOS .app must be signed.
This keeps the Dev Key out of the source upload to Extender.
"""
import argparse
import configparser
import io
from pathlib import Path
import shutil
import zipfile


def inject(config, credentials):
    settings = configparser.ConfigParser(interpolation=None, strict=False)
    settings.read_string(config.decode("utf-8"))
    if not settings.has_section("appsflyer"):
        settings.add_section("appsflyer")
    for name in ("key", "apple_app_id"):
        if credentials.has_option("appsflyer", name):
            settings.set("appsflyer", name, credentials.get("appsflyer", name))
    output = io.StringIO()
    settings.write(output)
    return output.getvalue().encode("utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--credentials", required=True, type=Path)
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    if args.output.exists():
        parser.error("Output already exists; choose a new path.")
    credentials = configparser.ConfigParser(interpolation=None)
    with args.credentials.open() as stream:
        credentials.read_file(stream)
    if not credentials.get("appsflyer", "key", fallback="").strip():
        parser.error("The credentials file must contain a nonempty [appsflyer] key.")

    if args.source.suffix == ".apk":
        with zipfile.ZipFile(args.source) as source:
            config = inject(source.read("assets/game.projectc"), credentials)
            # Create with owner-only permissions before writing the Dev Key.
            args.output.touch(mode=0o600, exist_ok=False)
            with zipfile.ZipFile(args.output, "w") as target:
                for info in source.infolist():
                    name = info.filename.upper()
                    if name.startswith("META-INF/") and (
                        name.endswith((".RSA", ".DSA", ".EC", ".SF"))
                        or name == "META-INF/MANIFEST.MF"
                    ):
                        continue
                    data = config if info.filename == "assets/game.projectc" else source.read(info)
                    target.writestr(info, data)
    elif args.source.suffix == ".app" and args.source.is_dir():
        config = inject((args.source / "game.projectc").read_bytes(), credentials)
        args.output.mkdir(mode=0o700)
        shutil.copytree(args.source, args.output, dirs_exist_ok=True)
        args.output.chmod(0o700)
        (args.output / "game.projectc").write_bytes(config)
    else:
        parser.error("Source must be an Android .apk or an iOS .app directory.")
    print("Test credentials inserted locally. Sign the output before installing.")


if __name__ == "__main__":
    main()

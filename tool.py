import argparse
import datetime
import os
import re
import shutil
import sys
import tempfile

import pip

C1 = "\033[34m"
C2 = "\033[32m"
C3 = "\033[33m"
C0 = "\033[0m"
LOGO = rf"""
{C1} ______     __         __         __  __   {C3} __   ______
{C1}/\  ___\   /\ \       /\ \       /\ \/\ \  {C3}/\_\ /\  ___\
{C1}\ \  __\   \ \ \____  \ \ \____  \ \ \_\ \ {C3}\/_/ \ \___  \
{C1} \ \_____\  \ \_____\  \ \_____\  \ \_____\{C3}      \/\_____\
{C1}  \/_____/   \/_____/   \/_____/   \/_____/{C3}       \/_____/ {C2}
 __    __     ______     _____     __  __     __         ______     ______
/\ "-./  \   /\  __ \   /\  __-.  /\ \/\ \   /\ \       /\  ___\   /\  ___\
\ \ \-./\ \  \ \ \/\ \  \ \ \/\ \ \ \ \_\ \  \ \ \____  \ \  __\   \ \___  \
 \ \_\ \ \_\  \ \_____\  \ \____-  \ \_____\  \ \_____\  \ \_____\  \/\_____\
  \/_/  \/_/   \/_____/   \/____/   \/_____/   \/_____/   \/_____/   \/_____/
{C0}"""


def log(level, text):
    # Log level colors
    LEVEL_COLORS = {
        "error": "\033[31m",
        "success": "\033[32m",
        "warning": "\033[33m",
        "info": "\033[34m",
        "debug": "\033[35m",
    }
    RESET_COLOR = "\033[0m"
    # Log level name
    LEVEL_NAME = {
        "error": "ERROR",
        "success": "SUCCESS",
        "warning": "WARNING",
        "info": "INFO",
        "debug": "DEBUG",
    }
    print(LEVEL_COLORS[level] + LEVEL_NAME[level] + ": " + text + RESET_COLOR)


def install_package(package):
    log("info", f"pip install {package} ...")
    output = ""
    with tempfile.TemporaryFile(mode="w+") as tempf:
        sys.stdout = tempf
        sys.stderr = tempf
        ret = pip.main(["install", package])
        sys.stdout = sys.__stdout__
        sys.stderr = sys.__stderr__
        tempf.seek(0)
        output = tempf.read()
    if ret != 0:
        log("error", f"failed to install {package}, see below for details:")
        print(output)
        exit(1)
    log("success", f"{package} successfully installed")


if os.name == "nt":  # Windows
    try:
        import curses  # noqa: F401
    except ImportError:
        install_package("windows-curses")

try:
    from kconfiglib import Kconfig
    from menuconfig import menuconfig as Kmenuconfig
except ImportError:
    install_package("kconfiglib")
    from kconfiglib import Kconfig
    from menuconfig import menuconfig as Kmenuconfig


C_FILE_TEMP = """/**
 * @file &&&FILE_NAME&&&.c
 * @brief &&&BREIF&&&
 * @author &&&AUTHOR&&& (&&&EMAIL&&&)
 * @version &&&VERSION&&&
 * @date &&&DATE&&&
 *
 * THINK DIFFERENTLY
 */

#include "&&&FILE_NAME&&&.h"

// Private Defines --------------------------

// Private Typedefs -------------------------

// Private Macros ---------------------------

// Private Variables ------------------------

// Public Variables -------------------------

// Private Functions ------------------------

// Public Functions -------------------------

// Source Code End --------------------------
"""

H_FILE_TEMP = """/**
 * @file &&&FILE_NAME&&&.h
 * @brief &&&BREIF&&&
 * @author &&&AUTHOR&&& (&&&EMAIL&&&)
 * @date &&&DATE&&&
 *
 * THINK DIFFERENTLY
 */

#ifndef __&&&FILE_NAME_UPPER&&&_H__
#define __&&&FILE_NAME_UPPER&&&_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "modules.h"

// Public Defines ---------------------------

// Public Typedefs --------------------------

// Public Macros ----------------------------

// Exported Variables -----------------------

// Exported Functions -----------------------

#ifdef __cplusplus
}
#endif

#endif /* __&&&FILE_NAME_UPPER&&&__ */

"""

README_TEMP = """# Module: &&&MODULE_NAME&&&

&&&MODULE_BRIEF&&&

## 1. Introduction

## 2. Notice

## 3. Usage

"""

KCONFIG_TEMP = """menu "&&&MODULE_NAME&&& Configuration"

endmenu
"""

AVAILABLE_TYPES = [
    "algorithm",
    "datastruct",
    "communication",
    "debug",
    "graphics",
    "nn",
    "peripheral",
    "storage",
    "system",
    "unittest",
    "utility",
]


def module_wizard():
    try:
        from rich.console import Console
        from rich.prompt import Confirm, Prompt
        from rich.table import Table
    except ImportError:
        install_package("rich")
        from rich.console import Console
        from rich.prompt import Confirm, Prompt
        from rich.table import Table

    module_root = os.path.dirname(os.path.abspath(__file__))
    con = Console()

    con.print(f"[blue]Module root: [green]{module_root}")

    module_type = Prompt.ask(
        "[yellow]Please select module type",
        choices=AVAILABLE_TYPES,
    )

    module_name = Prompt.ask("[yellow]Please input module name")
    module_name = module_name.strip().capitalize()

    module_brief = Prompt.ask("[yellow]Please input module brief")
    module_brief = module_brief.strip().capitalize()

    module_author = "Ellu" if os.getenv("AUTHOR") is None else str(os.getenv("AUTHOR"))
    module_email = (
        "ellu.grif@gmail.com" if os.getenv("EMAIL") is None else str(os.getenv("EMAIL"))
    )
    module_version = (
        "1.0.0" if os.getenv("VERSION") is None else str(os.getenv("VERSION"))
    )

    module_date = datetime.datetime.now().strftime("%Y-%m-%d")
    module_file_name = module_name.replace(" ", "_").lower()

    table = Table(show_header=False, box=None)

    table.add_row("[blue]Module name", ":", f"[green]{module_name}")
    table.add_row("[blue]Module brief", ":", f"[green]{module_brief}")
    table.add_row("[blue]Module type", ":", f"[green]{module_type}")
    table.add_row("[blue]Module date", ":", f"[green]{module_date}")
    table.add_row("[blue]Module filename", ":", f"[green]{module_file_name}.\\[ch]")
    table.add_row("[blue]Module author", ":", f"[green]{module_author}")
    table.add_row("[blue]Module email", ":", f"[green]{module_email}")
    table.add_row("[blue]Module version", ":", f"[green]{module_version}")

    con.print(table)

    conf = Confirm.ask("[yellow]Confirm to create module?", default=True)
    if not conf:
        sys.exit(0)

    create_readme_conf = Confirm.ask("[yellow]Create README.md?", default=False)
    create_kconfig_conf = Confirm.ask("[yellow]Create Kconfig?", default=False)

    # Create module folder
    module_path = os.path.join(module_root, module_type, module_file_name)
    if os.path.exists(module_path):
        con.print(f"[red]Module dir already exists: [yellow]{module_path}")
        conf_del = Confirm.ask(
            "[red]Purge existing module? [yellow](DANGER) ", default=False
        )
        if not conf_del:
            con.print("[red]Abort.")
            sys.exit(0)
        shutil.rmtree(module_path)
    os.makedirs(module_path)

    # Create module files
    c_file_path = os.path.join(module_path, module_file_name + ".c")
    h_file_path = os.path.join(module_path, module_file_name + ".h")

    c_file_content = (
        C_FILE_TEMP.replace("&&&FILE_NAME&&&", module_file_name)
        .replace("&&&BREIF&&&", module_brief)
        .replace("&&&AUTHOR&&&", module_author)
        .replace("&&&EMAIL&&&", module_email)
        .replace("&&&VERSION&&&", module_version)
        .replace("&&&DATE&&&", module_date)
    )
    with open(c_file_path, "w", encoding="utf-8") as f:
        f.write(c_file_content)

    h_file_content = (
        H_FILE_TEMP.replace("&&&FILE_NAME&&&", module_file_name)
        .replace("&&&BREIF&&&", module_brief)
        .replace("&&&AUTHOR&&&", module_author)
        .replace("&&&EMAIL&&&", module_email)
        .replace("&&&VERSION&&&", module_version)
        .replace("&&&DATE&&&", module_date)
        .replace("&&&FILE_NAME_UPPER&&&", module_file_name.upper())
    )
    with open(h_file_path, "w", encoding="utf-8") as f:
        f.write(h_file_content)

    if create_readme_conf:
        readme_file_path = os.path.join(module_path, "README.md")
        readme_content = README_TEMP.replace("&&&MODULE_NAME&&&", module_name).replace(
            "&&&MODULE_BRIEF&&&", module_brief
        )
        with open(readme_file_path, "w", encoding="utf-8") as f:
            f.write(readme_content)

    if create_kconfig_conf:
        kconfig_file_path = os.path.join(module_path, "Kconfig")
        kconfig_content = KCONFIG_TEMP.replace("&&&MODULE_NAME&&&", module_name)
        with open(kconfig_file_path, "w", encoding="utf-8") as f:
            f.write(kconfig_content)

    con.print("[green]Module created.")
    con.print(f"[green]C file at: [yellow]{c_file_path}")
    con.print(f"[green]H file at: [yellow]{h_file_path}")
    if create_readme_conf:
        con.print(f"[green]README file at: [yellow]{readme_file_path}")
    if create_kconfig_conf:
        con.print(f"[green]Kconfig file at: [yellow]{kconfig_file_path}")


def generate_config_file(conf_name, kconfig_file, config_in, config_out, header_out):
    kconf = Kconfig(kconfig_file, warn=False, warn_to_stderr=False)

    # Load config
    kconf.load_config(config_in)
    kconf.write_config(config_out)
    kconf.write_autoconf(header_out)

    with open(header_out, "r+") as header_file:
        content = header_file.read()

    # Preprocess the content
    content = content.replace("CONFIG_", "")
    content = content.replace("\\\\", "\\")
    #  remove standalone " and leave \" as "
    content = re.sub(r'(?<!\\)"', "", content)
    content = re.sub(r"\\\"", '"', content)

    with open(header_out, "w") as header_file:
        conf_name = conf_name.upper()
        header_file.write(f"#ifndef _{conf_name}_H_\n")
        header_file.write(f"#define _{conf_name}_H_\n\n")

        header_file.write("#ifdef __cplusplus\n")
        header_file.write('extern "C" {\n')
        header_file.write("#endif /* __cplusplus */\n\n")

        header_file.write(content)

        header_file.write("\n#ifdef __cplusplus\n")
        header_file.write("}\n")
        header_file.write("#endif /* __cplusplus */\n\n")
        header_file.write(f"#endif /* _{conf_name}_H_ */\n")
    log("success", "config file make success")


def menuconfig(kconfig_file="Kconfig", config_out=".config", header_dir=None):
    log("info", "loading menuconfig")
    try:
        Kmenuconfig(Kconfig(kconfig_file))
    except Exception as e:
        log("error", f"run menuconfig failed, see error and output below:\n{e}")
        exit(1)
    if not os.path.exists(config_out):
        log("warning", "menuconfig not complete (.config not found)")
        exit(1)
    target = "modules_config.h"
    if header_dir:
        target = os.path.join(header_dir, target)
    generate_config_file("MODULES_CONFIG", kconfig_file, config_out, config_out, target)


if __name__ == "__main__":
    print(LOGO)

    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument(
        "-m", "--menuconfig", action="store_true", help="Run menuconfig"
    )
    parser.add_argument(
        "-n",
        "--newmodule",
        action="store_true",
        help="Create a new module",
    )
    parser.add_argument(
        "-g",
        "--generate",
        action="store_true",
        help="Generate header file without menuconfig",
    )
    parser.add_argument(
        "-k",
        "--kconfig",
        default="Kconfig",
        help="Specify the kconfig file, default is Kconfig",
    )
    parser.add_argument(
        "-c",
        "--config",
        default=".config",
        help="Specify the menuconfig output file, default is .config",
    )
    parser.add_argument(
        "-d",
        "--headerdir",
        type=str,
        default=os.getenv("MOD_HEADER_DIR"),
        help="Specify the directory for the output header file, or use MOD_HEADER_DIR env variable",
    )
    args = parser.parse_args()

    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    if args.headerdir:
        if not os.path.isdir(args.headerdir):
            log("error", "header dir must be a existed directory")
            exit(1)
    if args.menuconfig:
        menuconfig(args.kconfig, args.config, args.headerdir)
    elif args.generate:
        target = "modules_config.h"
        if args.headerdir:
            target = os.path.join(args.headerdir, target)
        generate_config_file(
            "MODULES_CONFIG", args.kconfig, args.config, args.config, target
        )
    elif args.newmodule:
        module_wizard()
    else:
        parser.print_help()
        exit(1)

import click
import subprocess


@click.group()
def cli():
    pass


def _get_build_type(build_type) -> list:
    return ["-s", f"build_type={build_type}"]


@click.command()
@click.argument("os")
@click.option("--build-type", default="Release")
def build(os: str, build_type: str) -> None:
    profile = "conan/linux/llvm-20" if os == "linux" else "conan/darwin/llvm-20"
    args = [
        "conan", "build", ".",
        "--profile", profile,
        *_get_build_type(build_type),
        "--build", "missing"
    ]
    print(*args)
    subprocess.check_call(args)


@click.command()
@click.argument("os")
@click.option("--build-type", default="Release")
def install(os: str, build_type: str):
    profile = "conan/linux/llvm-20" if os == "linux" else "conan/darwin/llvm-20"
    args = [
        "conan", "install", ".",
        "--profile", profile,
        *_get_build_type(build_type),
        "--build", "missing"
    ]
    print(*args)
    subprocess.check_call(args)


cli.add_command(build)
cli.add_command(install)

if __name__ == '__main__':
    cli()

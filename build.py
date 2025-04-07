import click
import subprocess
import platform


@click.group()
def cli():
    pass


def _get_build_type(build_type) -> tuple[str]:
    return "-s", f"build_type={build_type}"


def _get_profile_from_os(os: str) -> tuple[str]:
    if os == "darwin" or os == "linux":
        return "--profile", f"conan/{os}/llvm-20"
    raise Exception(f"Unknown OS {os}")


def _conan_build_cmd():
    return "conan", "build", ".", "--build", "missing"


def _conan_install_cmd():
    return "conan", "install", ".", "--build", "missing"


def _run_cmd(*args, **kwargs) -> None:
    if "verbose" in kwargs:
        del kwargs["verbose"]
        print(*args)
    subprocess.check_call(*args, **kwargs)


@click.command()
@click.argument("os", default=platform.system().lower())
@click.option("--build-type", default="Release")
def build(os: str, build_type: str) -> None:
    _run_cmd([*_conan_build_cmd(), *_get_profile_from_os(os), *_get_build_type(build_type)], verbose=True)


@click.command()
@click.argument("os", default=platform.system().lower())
@click.option("--build-type", default="Release")
def install(os: str, build_type: str):
    _run_cmd([*_conan_install_cmd(), *_get_profile_from_os(os), *_get_build_type(build_type)], verbose=True)


cli.add_command(build)
cli.add_command(install)

if __name__ == '__main__':
    cli()

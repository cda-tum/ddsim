"""Nox sessions."""

from __future__ import annotations

import argparse
import os
import sys
from typing import TYPE_CHECKING

import nox

if TYPE_CHECKING:
    from collections.abc import Sequence

nox.options.sessions = ["lint", "tests"]

PYTHON_ALL_VERSIONS = ["3.8", "3.9", "3.10", "3.11", "3.12"]

BUILD_REQUIREMENTS = [
    "mqt.core @ git+https://github.com/cda-tum/mqt-core.git@installation-improvements",
    "scikit-build-core[pyproject]>=0.6.1",
    "setuptools_scm>=7",
    "pybind11>=2.11",
]

if os.environ.get("CI", None):
    nox.options.error_on_missing_interpreters = True


@nox.session(reuse_venv=True)
def lint(session: nox.Session) -> None:
    """Lint the Python part of the codebase using pre-commit.

    Simply execute `nox -rs lint` to run all configured hooks.
    """
    session.install("pre-commit")
    session.run("pre-commit", "run", "--all-files", *session.posargs)


def _run_tests(
    session: nox.Session,
    *,
    install_args: Sequence[str] = (),
    run_args: Sequence[str] = (),
    extras: Sequence[str] = (),
) -> None:
    posargs = list(session.posargs)
    env = {"PIP_DISABLE_PIP_VERSION_CHECK": "1"}

    if os.environ.get("CI", None) and sys.platform == "win32":
        env["SKBUILD_CMAKE_ARGS"] = "-T ClangCL"

    _extras = ["test", *extras]
    if "--cov" in posargs:
        _extras.append("coverage")
        posargs.append("--cov-config=pyproject.toml")

    # On Linux, `mqt-core` needs to be installed with `--no-binary` to avoid ABI
    # incompatibility issues caused by compiling with different GCC versions.
    if sys.platform == "linux":
        install_args = ["--no-binary", "mqt.core", *install_args]

    session.install("-v", *BUILD_REQUIREMENTS, *install_args, env=env)
    install_arg = f"-ve.[{','.join(_extras)}]"
    session.install("--no-build-isolation", install_arg, *install_args, env=env)
    session.run("pytest", *run_args, *posargs, env=env)


@nox.session(reuse_venv=True, python=PYTHON_ALL_VERSIONS)
def tests(session: nox.Session) -> None:
    """Run the test suite."""
    _run_tests(session)


@nox.session()
def minimums(session: nox.Session) -> None:
    """Test the minimum versions of dependencies."""
    _run_tests(
        session,
        install_args=["--constraint=test/python/constraints.txt"],
        run_args=["-Wdefault"],
    )
    session.run("pip", "list")


@nox.session(reuse_venv=True)
def docs(session: nox.Session) -> None:
    """Build the docs. Pass "--serve" to serve. Pass "-b linkcheck" to check links."""
    parser = argparse.ArgumentParser()
    parser.add_argument("--serve", action="store_true", help="Serve after building")
    parser.add_argument("-b", dest="builder", default="html", help="Build target (default: html)")
    args, posargs = parser.parse_known_args(session.posargs)

    if args.builder != "html" and args.serve:
        session.error("Must not specify non-HTML builder with --serve")

    extra_installs = ["sphinx-autobuild"] if args.serve else []
    # On Linux, `mqt-core` needs to be installed with `--no-binary` to avoid ABI
    # incompatibility issues caused by compiling with different GCC versions.
    install_args = ["--no-binary", "mqt.core"] if sys.platform == "linux" else []
    session.install(*BUILD_REQUIREMENTS, *extra_installs, *install_args)
    session.install("--no-build-isolation", "-ve.[docs]", *install_args)
    session.chdir("docs")

    if args.builder == "linkcheck":
        session.run("sphinx-build", "-b", "linkcheck", "source", "_build/linkcheck", *posargs)
        return

    shared_args = (
        "-n",  # nitpicky mode
        "-T",  # full tracebacks
        f"-b={args.builder}",
        "source",
        f"_build/{args.builder}",
        *posargs,
    )

    if args.serve:
        session.run("sphinx-autobuild", *shared_args)
    else:
        session.run("sphinx-build", "--keep-going", *shared_args)

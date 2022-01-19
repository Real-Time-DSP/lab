def build():
    from subprocess import run
    from shutil import rmtree, copytree
    from os.path import exists
    from pathlib import Path
    result = run("jupyter-book clean -a .",capture_output=True)
    print(result.stdout.decode("utf-8"))
    result = run("jupyter-book build --builder html .",capture_output=True)
    print(result.stdout.decode("utf-8"))
    if exists("docs"):
        rmtree("docs")
    copytree("_build/html","docs")
    Path("docs/.nojekyll").touch()
    copytree("_static","docs/_static",dirs_exist_ok=True)
    
if __name__ == '__main__':
    build()
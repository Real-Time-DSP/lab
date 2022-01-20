def build():
    from subprocess import run
    from shutil import rmtree, copytree, make_archive
    from os.path import exists
    from pathlib import Path
    from re import sub
    
    # Build the lab manual
    result = run("jupyter-book clean -a .",capture_output=True)
    print(result.stdout.decode("utf-8"))
    result = run("jupyter-book build --builder html .",capture_output=True)
    print(result.stdout.decode("utf-8"))
    
    # Copy to docs folder to publish to github pages
    if exists("docs"):
        rmtree("docs")
    copytree("_build/html","docs")
    Path("docs/.nojekyll").touch()
    copytree("_static","docs/_static",dirs_exist_ok=True)
    
    #Make another copy in the stm32h735gdk folder for utexas.edu webpage
    if exists("stm32h735gdk"):
        rmtree("stm32h735gdk")
    for path in Path("_build").rglob("*.html"):      
        with open(path, "r", encoding="utf-8") as htmlfile:
            lines = htmlfile.readlines()
        with open(path, "w", encoding="utf-8") as htmlfile:
            for line in lines:
                htmlfile.write(sub("http://users.ece.utexas.edu/~bevans/courses/realtime/", "../../../../", line))
    copytree("_build/html","stm32h735gdk")
    copytree("_static","stm32h735gdk/_static",dirs_exist_ok=True)
    make_archive('stm32h735gdk', 'zip', 'stm32h735gdk')
    
if __name__ == '__main__':
    build()
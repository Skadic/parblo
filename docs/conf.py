# Sources:
# https://itnext.io/daily-bit-e-of-c-modern-documentation-tools-9b96ba283732
# https://stackoverflow.com/questions/59990484/doxygensphinxbreatheexhale

from textwrap3 import dedent

# Basic configuration
project = 'parblo'
copyright = '2023, Etienne Palanga'
author = 'Etienne Palanga'

# Extensions to use
extensions = [ "breathe", "exhale", 'myst_parser' ]

# Configuration for the breathe extension
# Which directory to read the Doxygen output from
breathe_projects = {"parblo":"xml"}
breathe_default_project = "parblo"

exhale_args = {
    "containmentFolder": "./api",
    "doxygenStripFromPath": "../include",
    #"doxygenStripFromPath": "../src",
    "rootFileName": "library_root.rst",
    "rootFileTitle": "parblo API",
    "createTreeView":        True,
    # TIP: if using the sphinx-bootstrap-theme, you need
    "treeViewIsBootstrap": True,
}

# Configuration for the theme
html_theme = "sphinx_book_theme"
html_theme_options = {
    "repository_url": "https://github.com/Skadic/parblo",
    "use_repository_button": True,
}
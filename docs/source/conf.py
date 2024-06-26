# Configuration file for the Sphinx documentation builder.

# -- helper scripts for documentation generation
import sys
sys.path.append('../scripts/')

# -- Project information

project = 'hekate-gateway'
copyright = ''
author = 'Marius Schmid'

release = '0.1'
version = '0.1.0'

# -- General configuration

extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.intersphinx',
    'myst_parser',
    #'sphinxcontrib.drawio'

]

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

templates_path = ['_templates']

# -- Options for HTML output

html_theme = 'sphinx_rtd_theme'

# -- Options for EPUB output
epub_show_urls = 'footnote'

# -- generate documentation from uml model --
import create_objectives_table as ot
import create_risks_table as rt
ot.create_output("../papyrus/hekate/hekate.uml", "generated/objectives.md")
rt.create_output("../papyrus/hekate/hekate.uml", "generated/risks.md")

import xml.etree.ElementTree as ET
from mdutils.mdutils import MdUtils
from pathlib import Path


root = None
ns = {'xmi': 'http://www.omg.org/spec/XMI/20131001'}

def _get_risk_name(base_class_id):
    base_elements = root.findall(f".//*[@xmi:id='{base_class_id}']",ns)
    return base_elements[0].attrib['name']

def _get_risks():
    risks = []
    for child in root:
        if("Risk" in child.tag):
            risk = {}
            risk['name'] = ""
            risk['text'] = ""
            risk['occurrence'] = ""
            risk['severity'] = ""
            risk['kind'] = ""
            risk['id'] = child.attrib['{http://www.omg.org/spec/XMI/20131001}id']
            if 'text' in child.attrib:
                risk['text'] = child.attrib['text']
            if 'kind' in child.attrib:
                risk['kind'] = child.attrib['kind']
            
            risk['name'] = _get_risk_name(child.attrib['base_Class'])
            risks.append(risk)
    return risks


def _create_md_table(risks, output_md_filepath):
    mdFile = MdUtils(file_name=output_md_filepath,title='Risks')

    table = []
    header = ["Name", "Text", "Occurrence", "Severity" , "Kind"]
    table.extend(header)
    for risk in risks:
        table.extend([risk['name'],risk['text'],risk['occurrence'],risk['severity'],risk['kind']])
    mdFile.new_line()
    mdFile.new_table(columns=5, rows=len(risks)+1, text=table, text_align='center')
    mdFile.create_md_file()


def create_output(input_xml_filepath, output_md_filepath):
    Path(output_md_filepath).parent.mkdir(parents=True, exist_ok=True)
    global root
    tree = ET.parse(input_xml_filepath)
    root = tree.getroot()
    risks = _get_risks()
    _create_md_table(risks,output_md_filepath)


# create_output("../papyrus/hekate/hekate.uml", "risks.md")
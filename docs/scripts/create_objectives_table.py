import xml.etree.ElementTree as ET
from mdutils.mdutils import MdUtils
from pathlib import Path


root = None
ns = {'xmi': 'http://www.omg.org/spec/XMI/20131001'}

def _get_objective_name(base_class_id):
    base_elements = root.findall(f".//*[@xmi:id='{base_class_id}']",ns)
    return base_elements[0].attrib['name']

def _get_objective_stakeholder(stake_holder_id):
    base_elements = root.findall(f".//*[@xmi:id='{stake_holder_id}']",ns)
    base_classifier = base_elements[0].attrib['base_Classifier']
    stakeholders = root.findall(f".//*[@xmi:id='{base_classifier}']",ns)
    return stakeholders[0].attrib['name']


def _get_objectives():
    objectives = []
    for child in root:
        if("Objective" in child.tag):
            objective = {}
            objective['id'] = child.attrib['id']
            objective['text'] = child.attrib['text']
            objective['name'] = _get_objective_name(child.attrib['base_Class'])
            if 'Stakeholder' in child.attrib:
                objective['stakeholder'] = _get_objective_stakeholder(child.attrib['Stakeholder'])
            else:
                objective['stakeholder'] = ""
            objectives.append(objective)
    return objectives


def _create_md_table(objectives, output_md_filepath):
    mdFile = MdUtils(file_name=output_md_filepath,title='Objectives')

    table = []
    header = ["ID", "Stakeholder", "Name", "Description"]
    table.extend(header)
    for objective in objectives:
        table.extend([objective['id'],objective['stakeholder'],objective['name'],objective['text']])
    mdFile.new_line()
    mdFile.new_table(columns=4, rows=5, text=table, text_align='center')

    mdFile.create_md_file()



def create_output(input_xml_filepath, output_md_filepath):
    Path(output_md_filepath).parent.mkdir(parents=True, exist_ok=True)
    global root
    tree = ET.parse(input_xml_filepath)
    root = tree.getroot()
    objectives = _get_objectives()
    _create_md_table(objectives,output_md_filepath)
# print(objectives)



#!/usr/bin/env python3
"""Syntax-only Swift and project-file validation; NOT an SDK compiler."""
import pathlib,json,plistlib
import tree_sitter,tree_sitter_swift
from openstep_parser import OpenStepDecoder
root=pathlib.Path(__file__).resolve().parents[1]
parser=tree_sitter.Parser(tree_sitter.Language(tree_sitter_swift.language()))
results=[]
for file in sorted(root.glob('ios/**/*.swift')):
    tree=parser.parse(file.read_bytes());errors=[]
    def walk(node):
        if node.type=='ERROR' or node.is_missing:errors.append((node.type,list(node.start_point)))
        for child in node.children:walk(child)
    walk(tree.root_node)
    results.append({'file':str(file.relative_to(root)),'syntaxErrors':errors})
for file in [*root.glob('ios/**/*.plist'),*root.glob('ios/**/*.xcprivacy')]:plistlib.loads(file.read_bytes())
with (root/'GameSeagullNext.xcodeproj/project.pbxproj').open() as file:project=OpenStepDecoder.ParseFromFile(file)
results.append({'xcodeProjectObjects':len(project['objects']),'note':'Parsing only. Not Xcode compilation or SDK type checking.'})
(root/'artifacts/source-validation.json').write_text(json.dumps(results,indent=2))
assert not any(x.get('syntaxErrors') for x in results)
print('All Swift files parsed; plists and generated project parsed. No iOS compilation claimed.')

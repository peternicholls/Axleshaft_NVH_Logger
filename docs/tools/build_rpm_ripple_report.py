#!/usr/bin/env python3
"""Print the short Markdown report using the existing ReportLab document style."""
from pathlib import Path
import importlib.util
import re
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle, PageBreak
from reportlab.lib import colors
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('protocol_style',ROOT/'docs/tools/build_timing_protocol.py')
style=importlib.util.module_from_spec(spec);spec.loader.exec_module(style)
body=style.styles['BodyNVH'].clone('RippleBody');body.fontSize=9;body.leading=11.4;body.spaceAfter=5
heading=style.styles['SectionNVH'].clone('RippleHeading');heading.fontSize=12;heading.leading=15;heading.spaceBefore=8
source=ROOT/'docs/experiments/rpm-ripple-diagnosis.md'
dest=ROOT/'output/pdf/RPM-ripple-short-report-and-procedure.pdf'
lines=source.read_text().splitlines();story=[];i=0
while i<len(lines):
 line=lines[i].strip()
 if not line:i+=1;continue
 if line=='<!-- PAGE BREAK -->':story.append(PageBreak());i+=1;continue
 if line.startswith('# '):story.append(Paragraph(style.markup(line[2:]),style.styles['TitleNVH']));i+=1;continue
 if line.startswith('## '):story.append(Paragraph(style.markup(line[3:]),heading));i+=1;continue
 if line.startswith('|'):
  rows=[]
  while i<len(lines) and lines[i].startswith('|'):
   row=[x.strip() for x in lines[i].strip().strip('|').split('|')]
   if not all(re.fullmatch(r':?-+:?',x)for x in row):rows.append([Paragraph(style.markup(x),body)for x in row])
   i+=1
  widths=[90,140,140,145]if len(rows[0])==4 else [315,200]
  table=Table(rows,colWidths=widths,repeatRows=1)
  table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),colors.HexColor('#eaf3f6')),('GRID',(0,0),(-1,-1),.4,colors.HexColor('#cbd7dd')),('VALIGN',(0,0),(-1,-1),'TOP'),('TOPPADDING',(0,0),(-1,-1),4),('BOTTOMPADDING',(0,0),(-1,-1),4)]))
  story.extend([table,Spacer(1,7)]);continue
 parts=[line];i+=1
 while i<len(lines)and lines[i].strip()and not lines[i].startswith(('#','|','<!--')):parts.append(lines[i].strip());i+=1
 story.append(Paragraph(style.markup(' '.join(parts)).replace('**',''),body))
def footer(canvas,doc):
 canvas.setFont('Helvetica',8);canvas.setFillColor(colors.HexColor('#526674'))
 canvas.drawString(40,25,'RPM ripple | 0.6.2 | 26 October 2026 | Procedure prepared, not executed')
 canvas.drawRightString(555,25,str(doc.page))
dest.parent.mkdir(parents=True,exist_ok=True)
SimpleDocTemplate(str(dest),pagesize=(595,842),leftMargin=40,rightMargin=40,topMargin=35,bottomMargin=48,title='RPM ripple: short report and diagnosis procedure').build(story,onFirstPage=footer,onLaterPages=footer)
print(dest)

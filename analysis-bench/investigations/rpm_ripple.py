#!/usr/bin/env python3
"""Exploratory interval analysis. Needs existing NumPy/ReportLab; no file repairs."""
from pathlib import Path
import argparse
import csv
import hashlib
import importlib.util
import json
import statistics
import struct
import zlib

ROOT = Path(__file__).resolve().parents[2]
ARCHIVED_INPUTS = ROOT/'docs/evidence/2026-10-26-rpm-ripple-inputs'
DEFAULT_FILES = [
    'OPT_UNSET_cd77abcc_UP74_0001.BIN',
    'OPT_UNSET_e3fab4b9_UP471_0001.BIN',
    'OPT_UNSET_e3fab4b9_UP484_0001.BIN',
]


def inspect(path):
    import numpy as np
    if path.stat().st_size>4096*16:
        raise ValueError('Exploratory profile limit is 4096 events; no truncation applied')
    raw = path.read_bytes()
    spec = importlib.util.spec_from_file_location('optical_validator', ROOT / 'Stage1_Optical_Test/validate_optical.py')
    validator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(validator)
    count = len(raw)//16
    validator.validate(path, count, zlib.crc32(raw))
    events = list(struct.iter_unpack('<QQ',raw))
    if any(time>2**53-1 for _,time in events):
        raise ValueError('Timestamp exceeds exact numeric precision for this investigation')
    if len(events)<7:
        raise ValueError('At least seven events required for this exploratory profile')
    intervals = np.diff([time for _,time in events]).astype(float)
    index = np.arange(len(intervals),dtype=float)
    centred = index-index.mean()
    linear = np.column_stack([np.ones(len(index)),centred])
    quadratic = np.column_stack([linear,centred**2])
    trend = linear @ np.linalg.lstsq(linear,intervals,rcond=None)[0]
    residual = intervals-trend
    curved_residual = intervals-quadratic @ np.linalg.lstsq(quadratic,intervals,rcond=None)[0]
    def lag_corr(values,lag):
        left,right=values[:-lag],values[lag:]
        if np.std(left)<1e-6 or np.std(right)<1e-6:return None
        return float(np.corrcoef(left,right)[0,1])
    lags=[{'lag_events':lag,'pairs':len(intervals)-lag,'correlation':lag_corr(residual,lag)}
          for lag in range(1,len(intervals)//3+1)]
    phases=[]
    for phase in range(5):
        group=residual[np.arange(len(residual))%5==phase]
        phases.append({'phase':phase+1,'count':len(group),'mean_residual_us':float(group.mean())})
    return {
        'file':path.name,'sha256':hashlib.sha256(raw).hexdigest(),'crc32':f'{zlib.crc32(raw):08x}',
        'size':len(raw),'events':len(events),'interval_count':len(intervals),
        'structure':'pass','independent_completion_reference':'not supplied',
        'first_event_us':events[0][1],'last_event_us':events[-1][1],
        'event_span_s':(events[-1][1]-events[0][1])/1e6,
        'mean_interval_us':float(intervals.mean()),'overall_rpm_assuming_one_mark':60e6/float(intervals.mean()),
        'interval_cv_percent':float(intervals.std()/intervals.mean()*100),
        'interval_std_us':float(intervals.std()),
        'mean_raw_rpm':float(np.mean(60e6/intervals)),
        'std_raw_rpm':float(np.std(60e6/intervals)),
        'cv_raw_rpm_percent':float(np.std(60e6/intervals)/np.mean(60e6/intervals)*100),
        'min_interval_us':int(intervals.min()),'max_interval_us':int(intervals.max()),
        'five_event_approx_seconds':float(intervals.mean()*5/1e6),
        'lag5_linear_correlation':lag_corr(residual,5),'lag5_quadratic_correlation':lag_corr(curved_residual,5),
        'intervals_exactly_multiple_of_250us':int(sum(intervals%250==0)),
        'lags':lags,'phase5':phases,
        'records':[{'interval_number':i+1,'sequence_end':events[i+1][0],
                    'elapsed_end_us':events[i+1][1],'interval_us':int(value),
                    'raw_rpm_assuming_one_mark':60e6/float(value),
                    'linear_trend_us':float(trend[i]),'linear_residual_us':float(residual[i]),
                    'phase_mod5':i%5+1}for i,value in enumerate(intervals)]}


def write_figure(results,out):
    from reportlab.graphics.shapes import Drawing,String,Rect
    from reportlab.graphics.charts.lineplots import LinePlot
    from reportlab.graphics.charts.barcharts import VerticalBarChart
    from reportlab.graphics import renderSVG,renderPDF
    from reportlab.lib import colors
    d=Drawing(1050,930)
    d.add(Rect(0,0,1050,930,fillColor=colors.HexColor('#fcfbf7'),strokeColor=None))
    def label(x,y,text,size=11):d.add(String(x,y,text,fontName='Helvetica',fontSize=size,fillColor=colors.HexColor('#21343b')))
    label(35,900,'RPM ripple: structure in the raw intervals, cause not established',19)
    label(35,878,'All events retained. Straight-line trend removed only for these diagnostics. Five-event phase is an exploratory hypothesis.',10)
    for row,result in enumerate(results):
        y=650-row*277
        label(35,y+194,f"{result['file']}   |   {result['events']} detections   |   {result['overall_rpm_assuming_one_mark']:.2f} RPM assumed",12)
        line=LinePlot();line.x=80;line.y=y;line.width=555;line.height=153
        line.data=[[(r['interval_number'],r['linear_residual_us']/1000)for r in result['records']]]
        line.lines[0].strokeColor=colors.HexColor('#245b75');line.lines[0].strokeWidth=1.2
        line.xValueAxis.valueMin=1;line.xValueAxis.valueMax=result['interval_count']
        line.xValueAxis.labels.fontSize=8;line.yValueAxis.labels.fontSize=8
        line.xValueAxis.visibleGrid=True;line.yValueAxis.visibleGrid=True
        line.xValueAxis.gridStrokeColor=colors.HexColor('#e3e5df');line.yValueAxis.gridStrokeColor=colors.HexColor('#e3e5df')
        d.add(line);label(35,y+168,'Interval residual (ms); lower = faster',10)
        label(255,y-29,'Consecutive detection interval',9)
        bar=VerticalBarChart();bar.x=735;bar.y=y;bar.width=265;bar.height=153
        bar.data=[[p['mean_residual_us']/1000 for p in result['phase5']]]
        bar.categoryAxis.categoryNames=['1','2','3','4','5'];bar.categoryAxis.labels.fontSize=9;bar.valueAxis.labels.fontSize=8
        bar.bars[0].fillColor=colors.HexColor('#a34d22');bar.bars[0].strokeColor=None
        d.add(bar);label(735,y+168,'Mean residual (ms) by five-event phase',10)
        label(755,y-29,'Exploratory five-event phase (not angle)',9)
        corr=result['lag5_linear_correlation'];corr_text='undefined' if corr is None else f'{corr:.3f}'
        label(80,y-51,f"Lag 5 correlation: {corr_text}; {result['interval_count']-5} pairs. 5 mean intervals = {result['five_event_approx_seconds']:.3f} s.",10)
    label(35,22,'Each row has its own vertical scale. Short captures, unknown voltages/drive ratio. One event per turn assumed. Cause/significance unverified.',9)
    renderSVG.drawToFile(d,str(out/'rpm-ripple.svg'))
    renderPDF.drawToFile(d,str(out/'rpm-ripple.pdf'))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runs-dir',type=Path,default=ARCHIVED_INPUTS)
    parser.add_argument('--output-dir',type=Path,default=ROOT/'output/analysis/rpm-ripple')
    parser.add_argument('--files',nargs='+',default=DEFAULT_FILES)
    args=parser.parse_args();args.output_dir.mkdir(parents=True,exist_ok=True)
    results=[inspect(args.runs_dir/name)for name in args.files]
    summary={'method':'Unsmoothed intervals, linear detrend; lag5 selected exploratorily, quadratic detrend sensitivity check. Not a causal or statistical-significance test.',
             'hardware_context':{'motor':'user-reported 12 V DC motor','speed_control':'voltage adjusted; values not recorded',
             'drive':'motor pulley to measured shaft; ratio/belt type unknown'},'runs':results}
    (args.output_dir/'analysis.json').write_text(json.dumps(summary,indent=2,allow_nan=False)+'\n')
    for result in results:
        with (args.output_dir/(result['file']+'-intervals.csv')).open('w',newline='')as file:
            writer=csv.DictWriter(file,fieldnames=list(result['records'][0]),lineterminator='\n');writer.writeheader();writer.writerows(result['records'])
    write_figure(results,args.output_dir)
    for r in results:
        print(f"{r['file']}: {r['events']} events, {r['overall_rpm_assuming_one_mark']:.3f} RPM; lag5 r={r['lag5_linear_correlation']}, {r['interval_count']-5} pairs; 5T={r['five_event_approx_seconds']:.3f}s")
    print(args.output_dir)
if __name__=='__main__':main()

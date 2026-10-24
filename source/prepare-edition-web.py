"""Package edition-specific editors, matching DSP and installer releases. No network."""
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
import hashlib,json,shutil,re,subprocess,sys
FIX=Path(__file__).resolve().parent
ROOT=FIX.parent
ED=FIX/'editions'
OUT=FIX/'output/editions-0.8.0'
SITE=FIX/'github-pages'

def build():
    subprocess.run([sys.executable,'-X','utf8',str(FIX/'build-web.py')],check=True)
    # Remove only generated duplicates/bytecode from the previous packaging pass.
    for p in SITE.rglob('*.pyc'):
        assert SITE.resolve() in p.resolve().parents
        p.unlink()
    old=SITE/'source/editions'
    if old.exists():
        for p in old.iterdir():
            if p.is_file() and (ED/'shared'/p.name).is_file():
                assert SITE.resolve() in p.resolve().parents
                p.unlink()
    for kind in ['groovebox','sampler']:
        app=SITE/'webapp'/kind;shutil.copytree(SITE/'webapp/editor',app,dirs_exist_ok=True)
        (app/'package.json').write_text('{"type":"module"}\n',encoding='utf-8')
        shutil.copy2(OUT/f'{kind}.wasm',app/'zombie-core.wasm')
        names=json.loads((OUT/f'{kind}-presets.json').read_text())
        (app/'presets.js').write_text('export const PRESETS='+json.dumps(names)+';\n',encoding='utf-8')
        core=(app/'core.js').read_text(encoding='utf-8')
        core="export const EDITION="+json.dumps(kind)+";\n"+core
        core=core.replace('([3-7])\\.(0|[1-9]\\d*)$','([3-8])\\.(0|[1-9]\\d*)(?:\\/(groovebox|sampler))?$')
        core=core.replace("firmware 0.3.x–0.7.x",'firmware 0.3.x–0.8.x')
        core=core.replace('const minor=Number(version[1]),patch=Number(version[2]);',"const minor=Number(version[1]),patch=Number(version[2]);this.edition=version[3]||'core';if(this.edition!==EDITION)throw Error('Open the '+this.edition+' editor for this firmware.');")
        core=core.replace('this.sampler=minor>=7;','this.sampler=minor>=7&&this.edition!==\'groovebox\';')
        core=core.replace('b.set([s.recMode,s.recQuant,s.metronome,s.fxAmount,s.fxRate],3444);','b.set([s.recMode,s.recQuant,s.metronome,s.fxAmount,s.fxRate],3444);b[3449]=EDITION===\'sampler\'?2:1;')
        core=core.replace("for(const t of s.tracks){if(!Array.isArray(t.p)","for(const t of s.tracks){if(Array.isArray(t.p)&&((EDITION==='groovebox'&&t.p[0]>7)||(EDITION==='sampler'&&t.p[0]!==8)))throw Error('Project belongs to another edition');if(!Array.isArray(t.p)")
        (app/'core.js').write_text(core,encoding='utf-8')
        html=(app/'index.html').read_text(encoding='utf-8').replace('FM-1 STUDIO / 0.7.4',kind.upper()+' / 0.8.0').replace('Zombie Core · FM-1 Studio','Zombie '+kind.title()+' · FM-1')
        js=(app/'app.js').read_text(encoding='utf-8')
        js=js.replace("'zcore-draft-scratch'","'zcore-"+kind+"-scratch'").replace("'zcore-draft-v3'","'zcore-"+kind+"-v3'")
        js=js.replace('0.7.4','0.8.0')
        extra_ru="headline:'Грув начинается здесь.',intro:'Шесть дорожек, 64 синтезаторных пресета и пианоролл. Новый движок Groovebox 0.8.0.',sound:'Синтез',sequence:'Пианоролл',pianoHelp:'Drag: длина · Shift: аккорд · Alt: удалить · Стрелки / Space / + / −'" if kind=='groovebox' else "headline:'Режь. Играй. Собирай.',intro:'32 пэда, нарезка, реверс и четыре слота своих WAV. Отдельная прошивка Sampler 0.8.0.',sound:'Тон / FX'"
        extra_en="headline:'Start with a groove.',intro:'Six tracks, 64 synthesis presets and a piano roll. Groovebox 0.8.0.',sound:'Synthesis',sequence:'Piano roll',pianoHelp:'Drag: length · Shift: chord · Alt: delete · Arrows / Space / + / −'" if kind=='groovebox' else "headline:'Chop. Play. Build.',intro:'32 pads, chopping, reverse and four user WAV slots. Dedicated Sampler 0.8.0 firmware.',sound:'Tone / FX'"
        js=js.replace('const dictionaries={ru:RU,en:EN};',"Object.assign(RU,{"+extra_ru+",midiHelp:'Открой редактор своей версии прошивки: "+kind+" 0.8.0. Разреши MIDI SysEx и закрой DAW.',alpha:'Новые сборки 0.8.0 проверены host/WASM-тестами; веб-установка и экран на настоящем FM-1 разработчиком пока не проверены.'});\nObject.assign(EN,{"+extra_en+",midiHelp:'Use the matching "+kind+" 0.8.0 firmware. Allow MIDI SysEx and close your DAW.',alpha:'0.8.0 builds are checked with host/WASM tests; physical FM-1 flashing and display are not verified by the developer.'});\nconst dictionaries={ru:RU,en:EN};")
        if kind=='groovebox':
            js=js.replace('...MODELS.map((s,i)=>new Option(s,String(i)))','...MODELS.slice(0,8).map((s,i)=>new Option(s,String(i)))')
            js=js.replace('value=Math.max(schema[id].min,', 'if(id===0)value=Math.min(7,value);value=Math.max(schema[id].min,',1)
            js=js.replace("fillControls('osc-controls',[1,2,3,10,11,19,20,25]);","fillControls('osc-controls',[1,2,3,10,11,25]);")
            js=js.replace("function renderSteps(){", "function renderSteps(){drawPianoRoll($('piano-roll'),state,selectedStep,bank,pianoHead);")
            js=js.replace("function showPosition(position){", "function showPosition(position){pianoHead=playing?position:-1;drawPianoRoll($('piano-roll'),state,selectedStep,bank,pianoHead);")
            js="import {drawPianoRoll,bindPianoRoll} from './piano.js';\nlet pianoHead=-1;\n"+js
            js+='''
bindPianoRoll($('piano-roll'),{getState:()=>state,getSelected:()=>selectedStep,getBank:()=>bank,status,
 select:at=>perform(async()=>{await setPanel(5,at);selectedStep=at;bank=Math.floor(at/16);renderSteps();}),
 edit:change=>perform(async()=>{if(state.recording)throw Error(lang==='ru'?'Сначала выключи REC.':'Stop REC before editing.');const {at,note,duration,add,remove}=change,s=state.tracks[state.selected].steps[at];selectedStep=at;await setPanel(5,at);
 if(change.toggle)s.velocity=s.velocity?0:100;
 else if(remove){if(s.note===note){const chord=s.chord.filter(Boolean);if(chord.length){s.note=chord.shift()-1;s.chord=[...chord,0,0,0].slice(0,3);}else s.velocity=0;}else s.chord=s.chord.map(n=>n===note+1?0:n);}
 else if(add&&s.velocity&&note!==s.note){if(!s.chord.includes(note+1)){const i=s.chord.indexOf(0);if(i<0)throw Error('Max 4 notes per step');s.chord[i]=note+1;}}
 else{s.note=note;s.velocity=s.velocity||100;}if(duration!==null)s.duration=duration;await writeStep(state.selected,at);renderSteps();})});
setTab('sequence');
'''
            html=html.replace('<div id="step-grid">','<h3>Piano Roll</h3><p class="muted" data-i="pianoHelp"></p><canvas id="piano-roll" width="1100" height="310" tabindex="0" style="height:310px;touch-action:none"></canvas><div id="step-grid">')
            html=html.replace('<button data-tab="samples" data-i="samples"></button>','')
            shutil.copy2(ED/'piano.js',app/'piano.js')
        else:
            js=js.replace('...MODELS.map((s,i)=>new Option(s,String(i)))','new Option("SAMPLE","8")')
            js=js.replace('value=Math.max(schema[id].min,','if(id===0)value=8;value=Math.max(schema[id].min,',1)
            js+="\nsetTab('samples');\n"
            sampler=(app/'sampler.js').read_text(encoding='utf-8');start=sampler.index(" $('sampler-enable').onclick=");end=sampler.index(' for(const bank',start)
            sampler=sampler[:start]+" $('sampler-enable').disabled=true;\n"+sampler[end:]
            sampler=sampler.replace("$('sampler-enable').disabled=!available();","$('sampler-enable').disabled=true;").replace('EDIT → страница 4/4','HOME / EDIT').replace('EDIT → page 4/4','HOME / EDIT')
            (app/'sampler.js').write_text(sampler,encoding='utf-8')
            html=html.replace('</head>','<style>#osc-controls,.keyboard{display:none}#sound .sound-grid>.card:first-child{display:none}#model{pointer-events:none}</style></head>')
        html=html.replace('9 MODELS','8 MODELS' if kind=='groovebox' else 'PAD SAMPLER')
        (app/'index.html').write_text(html,encoding='utf-8');(app/'app.js').write_text(js,encoding='utf-8')
        css=(app/'interface.css').read_text(encoding='utf-8')+'\nbody[data-theme=afterglow] #banks button[aria-selected=true],body[data-theme=afterglow] .pad-banks button[aria-pressed=true]{background:var(--accent);color:var(--ink)}\n#sampler-enable:disabled{opacity:1;color:var(--accent);background:var(--panel);border-color:var(--line)}\n'
        (app/'interface.css').write_text(css,encoding='utf-8')
    downloads=SITE/'downloads';releases=json.loads((SITE/'releases.json').read_text(encoding='utf-8'))
    extra=[]
    for kind,prefix in [('groovebox',63),('sampler',64)]:
        file=OUT/f'zombie-{kind}-0.8.0.fwsc';shutil.copy2(file,downloads/file.name);raw=file.read_bytes()
        note={'ru':'Синтез, 64 пресета, пианоролл и стартовый грув. Standalone при запуске. Новая сборка: проверена в симуляторе, на железе пока нет.','en':'Synthesis, 64 presets, piano roll and starter groove. Standalone at startup. New build: simulator-tested, not hardware-verified.'} if kind=='groovebox' else {'ru':'32 пэда, свои WAV, CHOP, reverse, GATE / LOOP. Standalone при запуске. Новая сборка: проверена в симуляторе, на железе пока нет.','en':'32 pads, user WAVs, CHOP, reverse, GATE / LOOP. Standalone at startup. New build: simulator-tested, not hardware-verified.'}
        extra.append(dict(product=f'FM-1_{prefix}000800',version='0.8.0',label={'ru':kind.title()+' 0.8.0 · отдельная прошивка','en':kind.title()+' 0.8.0 · dedicated firmware'},notes=note,bytes=len(raw),sha256=hashlib.sha256(raw).hexdigest(),file='downloads/'+file.name,editor=f'webapp/{kind}/'))
    releases['current']=extra[0]['product'];releases['releases']=extra+releases['releases'];releases['releases'][2]['label']={'ru':'Core 0.7.4 · прежняя сборка','en':'Core 0.7.4 · previous build'}
    (SITE/'releases.json').write_text(json.dumps(releases,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    js=(SITE/'installer.js').read_text(encoding='utf-8');js=js.replace("if(release)$('package-badge').textContent=release.version;","if(release){$('package-badge').textContent=release.version;for(const a of document.querySelectorAll('a[data-t=editor],a[data-t=tryEditor]'))a.href=release.editor||'webapp/editor/';}")
    js=js.replace("if(!release)throw Error('Unknown release');","if(!release)throw Error('Unknown release');render();")
    (SITE/'installer.js').write_text(js,encoding='utf-8')
    html=(SITE/'index.html').read_text(encoding='utf-8').replace('WEB 1.0 · CORE 0.7.4','WEB 1.1 · GROOVEBOX / SAMPLER 0.8.0').replace('href="downloads/firmware-source-0.7.4.zip"','href="downloads/editions-source-0.8.0.zip"')
    html=html.replace('<div class="features">','<p class="row"><a href="webapp/groovebox/">Groovebox / Piano Roll ↗</a><a href="webapp/sampler/">Sampler / Pads ↗</a></p><div class="features">')
    html=html.replace('<section class="bottom-notes">','<section class="bottom-notes"><article><h2>Groovebox / FM-1</h2><img src="previews/groovebox-piano.png" alt="Пианоролл FM-1 из host-симулятора" width="240" height="240"><p class="muted">6 tracks · 64 synth presets · 4-note piano roll</p></article><article><h2>Sampler / FM-1</h2><img src="previews/sampler-pads.png" alt="Пэды FM-1 из host-симулятора" width="240" height="240"><p class="muted">32 pads · 4 user slots · chop / reverse / loop</p><a href="downloads/Zombie-FM1-DAW-Profiles-0.8.0.zip" download>FL Studio + Ableton Live · scripts / RU + EN</a></article></section><section class="bottom-notes">')
    (SITE/'index.html').write_text(html,encoding='utf-8')
    html=(SITE/'index.html').read_text(encoding='utf-8').replace('src="installer.js"','src="installer.js?v=1.1"')
    (SITE/'index.html').write_text(html,encoding='utf-8')
    preview=SITE/'previews';preview.mkdir(exist_ok=True)
    for src,name in [('groovebox/groovebox-piano-ru.png','groovebox-piano.png'),('sampler/sampler-home-ru.png','sampler-pads.png')]:shutil.copy2(OUT/src,preview/name)
    for kind in ['groovebox','sampler']:shutil.copy2(OUT/(kind+'-native.gif'),preview/(kind+'-motion.gif'))
    shutil.copy2(OUT/'groovebox/groovebox-demo.wav',preview/'groovebox-demo.wav')
    html=(SITE/'index.html').read_text(encoding='utf-8').replace('6 tracks · 64 synth presets · 4-note piano roll','6 tracks · 64 synth presets · 4-note piano roll<br>Экраны из симулятора / simulator preview<br><a href="previews/groovebox-motion.gif">Animation ↗</a> · <a href="previews/groovebox-demo.wav">Groove demo ↗</a>').replace('32 pads · 4 user slots · chop / reverse / loop','32 pads · 4 user slots · chop / reverse / loop<br>Экраны из симулятора / simulator preview<br><a href="previews/sampler-motion.gif">Animation ↗</a>')
    (SITE/'index.html').write_text(html,encoding='utf-8')
    source=SITE/'source/editions/shared';source.mkdir(parents=True,exist_ok=True)
    for p in (ED/'shared').iterdir():
        if p.is_file():shutil.copy2(p,source/p.name)
    for name in ['build-editions.py','prepare-edition-web.py']:shutil.copy2(FIX/name,SITE/'source'/name)
    shutil.copy2(ED/'presets.json',SITE/'source/editions/presets.json');shutil.copytree(ED/'profiles',SITE/'source/profiles',dirs_exist_ok=True,ignore=shutil.ignore_patterns('__pycache__'))
    for kind in ['groovebox','sampler']:
        path=SITE/'source/config'/kind;path.mkdir(parents=True,exist_ok=True)
        for name in ['edition.h','fm1.toml']:shutil.copy2(ED/kind/name,path/name)
    archive=downloads/'editions-source-0.8.0.zip'
    with ZipFile(FIX/'output/Zombie-Core-0.7.4-Standalone.zip') as old,ZipFile(archive,'w',ZIP_DEFLATED) as z:
        for info in old.infolist():
            if info.filename.startswith(('source/platform/','audio-provenance/')):z.writestr(info,old.read(info))
        for directory,prefix in [(ED/'shared','editions/shared'),(ED/'assets','editions/assets'),(ED/'profiles','editions/profiles')]:
            for p in directory.rglob('*'):
                if p.is_file() and '__pycache__' not in p.parts:z.write(p,'source/'+prefix+'/'+p.relative_to(directory).as_posix())
        for kind in ['groovebox','sampler']:
            for name in ['edition.h','fm1.toml']:z.write(ED/kind/name,'source/editions/'+kind+'/'+name)
        for name in ['presets.json','piano.js']:z.write(ED/name,'source/editions/'+name)
        for name in ['build-editions.py','prepare-edition-web.py']:z.write(FIX/name,'source/'+name)
        z.write(FIX/'zombie-core/LICENSE','LICENSE')
    with ZipFile(downloads/'Zombie-FM1-DAW-Profiles-0.8.0.zip','w',ZIP_DEFLATED) as z:
        for p in (ED/'profiles').rglob('*'):
            if p.is_file() and '__pycache__' not in p.parts:z.write(p,p.relative_to(ED/'profiles').as_posix())
    (SITE/'licenses/THIRD-PARTY.txt').write_text((SITE/'licenses/THIRD-PARTY.txt').read_text(encoding='utf-8')+'\nGroovebox/Sampler 0.8.0 matching source: ../downloads/editions-source-0.8.0.zip.\nOriginal application, piano roll, display animations and DAW profiles: MIT.\n',encoding='utf-8')
    (SITE/'README.md').write_text('''# Zombie FM-1 · Groovebox / Sampler 0.8.0

Site / installer: https://joolo09.github.io/zombie-fm1-web/

Two dedicated firmware packages. Groovebox: six synthesis tracks, 64 original
presets, native/web four-note piano roll and starter groove. Sampler: 32 pads,
four user PCM slots, chopping, reverse, choke groups and one-shot/gate/loop.
Both boot Standalone with local sound; HOME + SEL cycles Standalone → DAW Synth
→ DAW Controller for the current session. Never automatically flashes a device.

Original 240x240 RGB565 device UI: track colors, output meter, page transitions,
fading pad hits, moving sequencer playhead. English and Russian labels.
Displayed previews are from the host simulator, not physical hardware photos.

Find FM-1, then Install in Chrome/Edge with MIDI SysEx. Back up projects/WAVs
and close the DAW / other MIDI tabs first. Firmware updates are validated with
size, SHA-256 and package identity; success requires the restarted identity.

Dedicated FL Studio and Ableton Live 11/12 Python 3 profiles are in
source/profiles/ and downloads/Zombie-FM1-DAW-Profiles-0.8.0.zip. Read the included
RU/EN guide for installation, pad note maps and manual MIDI Learn in FL.

Both firmware binaries compile; host, actual WASM audio and stub DAW API tests
pass. Physical FM-1 flashing/display and actual FL/Live sessions are not verified
by the developer. Older 0.7.4 / 0.7.1 downloads remain available.

Editable new firmware source: source/editions/. Complete matching platform,
generators/assets and profiles: downloads/editions-source-0.8.0.zip.
Application/DSP/UI/profiles: MIT. Hardware/boot/update platform: MPL-2.0.
Minecraft audio in Sampler: Mojang/Microsoft, provenance in source archive.
GitHub Pages: main, / (root), HTTPS. Paths support a repository subdirectory.
''',encoding='utf-8')
    for dest in [ROOT/'output/zombie-core-site',FIX/'output/site']:
        shutil.copytree(SITE,dest,dirs_exist_ok=True,ignore=shutil.ignore_patterns('source','tests','package.json','README.md','__pycache__'))
    with ZipFile(OUT/'Zombie-FM1-Editions-0.8.0.zip','w',ZIP_DEFLATED) as z:
        for p in downloads.iterdir():
            if p.is_file() and ('0.8.0' in p.name or '0.7.4.fwsc' in p.name):z.write(p,p.name)
    print('Two web editors, firmware selector, DAW scripts, source and local package ready.')

if __name__=='__main__':build()

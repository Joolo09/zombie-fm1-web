/* SPDX-License-Identifier: MIT — same GM map as native drums.inc. */
export const DRUMS=[
 [36,'Kick','Кик'],[38,'Snare','Снейр'],[42,'Closed hat','Закр. хэт'],[46,'Open hat','Откр. хэт'],
 [39,'Clap','Клэп'],[37,'Rim','Рим'],[45,'Low tom','Низкий том'],[47,'Mid tom','Средний том'],
 [50,'High tom','Высокий том'],[49,'Crash','Крэш'],[51,'Ride','Райд'],[56,'Cowbell','Ковбелл'],
 [54,'Tambourine','Тамбурин'],[70,'Shaker','Шейкер'],[75,'Claves','Клавес'],[76,'Wood','Вуд']
];
export const drumIndex=note=>{const i=DRUMS.findIndex(d=>d[0]===note);return i<0?note%16:i;};
export const drumName=(note,lang='en')=>DRUMS[drumIndex(note)][lang==='ru'?2:1];

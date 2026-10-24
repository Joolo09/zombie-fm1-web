/* SPDX-License-Identifier: MIT — package validation before requesting MIDI access. */
import {productOf, logicalImage} from './vendor/fm1pkg.js';

export async function checkPackage(raw, meta) {
  if (!(raw instanceof Uint8Array) || !Number.isSafeInteger(meta.bytes) || raw.length !== meta.bytes || raw.length < 960)
    throw Object.assign(new Error('Package size does not match the release'), {code:'badpkg'});
  const digest = [...new Uint8Array(await crypto.subtle.digest('SHA-256', raw))]
    .map(b=>b.toString(16).padStart(2,'0')).join('');
  if (digest !== meta.sha256 || !/^FM-1_\d+$/.test(meta.product) || productOf(raw) !== meta.product)
    throw Object.assign(new Error('Package SHA-256 or device identity does not match'), {code:'badpkg'});
  const marker = new TextEncoder().encode('FELUCCA-LOADER-1');
  const hasLoader = raw.some((b,i)=>b===marker[0] && marker.every((v,j)=>raw[i+j]===v));
  if (!hasLoader) throw Object.assign(new Error('FM-1 loader is missing'), {code:'badpkg'});
  return {image:logicalImage(raw), product:meta.product};
}

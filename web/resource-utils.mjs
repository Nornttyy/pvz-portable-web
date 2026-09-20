const RESOURCE_ROOTS = new Set(['images', 'reanim', 'particles', 'sounds', 'properties', 'data']);
export const LIMITS = Object.freeze({ files: 6000, fileBytes: 32 * 1048576, totalBytes: 128 * 1048576, saves: 16 * 1048576 });

export function safeRelativePath(path) {
  if (typeof path !== 'string' || !path || path.length > 240 || !/^[A-Za-z0-9_./ -]+$/.test(path)) throw Error('不安全的文件路径');
  if (path.startsWith('/') || path.split('/').some(x => !x || x === '.' || x === '..')) throw Error('不安全的文件路径');
  return path;
}

export function validateManifest(manifest) {
  if (manifest?.schema !== 1 || (manifest.delivery !== 'bundled' && manifest.localOnly !== true) || !Array.isArray(manifest.files)) throw Error('资源清单格式不正确');
  if (manifest.files.length < 1 || manifest.files.length > LIMITS.files || manifest.totalFiles !== manifest.files.length) throw Error('资源文件数量不正确');
  const bundleUrl = manifest.delivery === 'bundled' ? `resources/game-${manifest.bundle?.sha256?.slice(0,12)}.zip` : '/local-resources.zip';
  if (manifest.bundle?.url !== bundleUrl || !/^[a-f0-9]{64}$/.test(manifest.bundle?.sha256 || '')) throw Error('资源包地址或校验值不正确');
  if (!Number.isSafeInteger(manifest.bundle.size) || manifest.bundle.size < 1 || manifest.bundle.size > LIMITS.totalBytes) throw Error('资源包大小不正确');
  const seen = new Set();
  let size = 0;
  for (const entry of manifest.files) {
    safeRelativePath(entry.path);
    if (!RESOURCE_ROOTS.has(entry.path.split('/')[0])) throw Error('资源文件目录不正确');
    const key = entry.path.toLowerCase();
    if (seen.has(key)) throw Error('资源清单包含重名文件');
    seen.add(key);
    if (!Number.isSafeInteger(entry.size) || entry.size < 0 || entry.size > LIMITS.fileBytes) throw Error('单个资源文件过大');
    if (!/^[a-f0-9]{64}$/.test(entry.sha256)) throw Error('文件校验值不正确');
    size += entry.size;
  }
  if (size !== manifest.totalBytes || size > LIMITS.totalBytes) throw Error('资源总大小不正确');
  for (const path of ['properties/resources.xml', 'properties/lawnstrings.txt', 'images/background1.jpg', 'reanim/peashooter.reanim', 'reanim/zombie.reanim']) {
    if (!seen.has(path)) throw Error(`缺少关键资源：${path}`);
  }
  return manifest;
}

export async function sha256(bytes) {
  const hash = await crypto.subtle.digest('SHA-256', bytes);
  return [...new Uint8Array(hash)].map(x => x.toString(16).padStart(2, '0')).join('');
}

export function writeVirtualFile(FS, base, relative, data) {
  safeRelativePath(relative);
  const parts = relative.split('/');
  let parent = base;
  if (!FS.analyzePath(parent).exists) FS.mkdir(parent);
  for (const part of parts.slice(0, -1)) {
    parent += '/' + part;
    if (!FS.analyzePath(parent).exists) FS.mkdir(parent);
  }
  FS.writeFile(base + '/' + relative, data);
}

export function normalizeSavePath(raw) {
  const path = safeRelativePath(raw);
  const parts = path.split('/');
  if (parts[0] === 'saves') parts.shift();
  if (parts[0] === 'userdata' && parts.length === 2 && /^[A-Za-z0-9_-]+\.(?:dat|v4|txt)$/.test(parts[1])) return parts.join('/');
  if (parts.length === 1 && parts[0] === 'registry.regemu') return parts[0];
  throw Error('备份内含不支持的存档文件');
}

export function listVirtualFiles(FS, dir) {
  if (!FS.analyzePath(dir).exists) return [];
  const files = [];
  for (const name of FS.readdir(dir)) {
    if (name === '.' || name === '..') continue;
    const path = dir + '/' + name;
    const info = FS.stat(path);
    if (FS.isDir(info.mode)) files.push(...listVirtualFiles(FS, path));
    else files.push(path);
  }
  return files;
}

// Legacy PopCap PAK: LE header, file table, 0x80 terminator, payload; XOR 0xf7.
// Using its native case-insensitive index is essential for split JPG/alpha PNGs.
export function createPakBuilder(entries) {
  const encoder = new TextEncoder();
  const names = new Set();
  let headerSize = 9;
  let payloadSize = 0;
  const table = entries.map(entry => {
    const path = safeRelativePath(entry.path);
    const name = encoder.encode(path.replaceAll('/', '\\'));
    if (name.length > 255 || names.has(path.toUpperCase())) throw Error('PAK 文件名无效或重名');
    if (!Number.isSafeInteger(entry.size) || entry.size < 0 || entry.size > LIMITS.fileBytes) throw Error('PAK 文件大小无效');
    names.add(path.toUpperCase());
    headerSize += 14 + name.length;
    payloadSize += entry.size;
    return { path, name, size: entry.size };
  });
  if (!table.length || table.length > LIMITS.files || payloadSize > LIMITS.totalBytes) throw Error('PAK 资源包过大');
  const bytes = new Uint8Array(headerSize + payloadSize);
  const view = new DataView(bytes.buffer);
  view.setUint32(0, 0xbac04ac0, true);
  view.setUint32(4, 0, true);
  let offset = 8;
  for (const entry of table) {
    bytes[offset++] = 0;
    bytes[offset++] = entry.name.length;
    bytes.set(entry.name, offset); offset += entry.name.length;
    view.setUint32(offset, entry.size, true); offset += 4;
    // A zero FILETIME is valid; timestamps do not affect image loading.
    offset += 8;
  }
  bytes[offset++] = 0x80;
  if (offset !== headerSize) throw Error('PAK 索引大小错误');
  let next = 0;
  let finished = false;
  return {
    append(path, data) {
      const expected = table[next];
      if (finished || !expected || path !== expected.path || data.length !== expected.size) throw Error('PAK 文件顺序或大小错误');
      bytes.set(data, offset); offset += data.length; next++;
    },
    finish() {
      if (finished || next !== table.length || offset !== bytes.length) throw Error('PAK 文件未完成');
      for (let i = 0; i < bytes.length; i++) bytes[i] ^= 0xf7;
      finished = true;
      return bytes;
    },
  };
}

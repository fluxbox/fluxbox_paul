local file = ...;
local keymodes = {};
local failed = false;

local function process_line(line)
    if string.match(line, '^%s*$') ~= nil then
        return line;
    end;
    local comment = string.match(line, '^%s*[#!](.*)$');
    if comment ~= nil then
        return '--' .. comment;
    end;

    local mode, key, cmd = string.match(line, '^%s*(%a%w*):%s*([^:]-)%s*:(.*)$');
    if key == nil then
        key, cmd = string.match(line, '^%s*([^:]*):(.*)$');
    end;

    if mode == nil or mode == 'default' then
        mode = 'default_keymode';
    else
        keymodes[mode] = true;
    end;

    if key ~= nil then
        return string.format('%s[%q] = %q', mode, key, cmd);
    else
        failed = true;
        return '-- FBCV16 ' .. line;
    end;
end;

file = string.gsub(file, '[^\n]*', process_line);

local decls = '';
for k, v in pairs(keymodes) do
    decls = decls .. k .. ' = newKeyMode();\n';
end;

if failed == true then
    decls = [[
--fluxbox-update_configs could not convert some of the lines into the new format.
--These lines are marked with FBCV16 and you will have to convert them yourself.

]] .. decls;
end;

return decls .. '\n' .. file;

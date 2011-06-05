read_resource, write_resource, res_magic = ...;

local cat_magic = {};

local function myerror(table, msg)
    error(getmetatable(table)._fullname .. ': ' .. msg);
end;

local function check_arg(table, key)
    if type(key) ~= 'string' then
        myerror(table, 'expecting strings as keys.');
    end;
    if string.match(key, "^_") then
        myerror(table, 'resource names must not begin with "_".');
    end;
    local t = getmetatable(table)[key];
    return t, getmetatable(t);
end;

local new_cat;
local function newindex(table, key, value)
    local meta = getmetatable(table);
    local t, mt = check_arg(table, key);

    if type(value) == 'table' then
        if mt == res_magic then
            myerror(table, '"' .. key .. '" is a resource.');
        end;
        if mt == nil or mt._magic ~= cat_magic then
            t = new_cat(table, key);
        end;
        for k,v in pairs(value) do
            t[k] = v;
        end;
    else
        if mt ~= nil and mt._magic == cat_magic and mt._state == 1 then
            myerror(table, '"' .. key .. '" is a category.');
        elseif mt == res_magic then
            write_resource(t, value);
        else
            meta[key] = value;
        end;
    end;
end;

local function index(table, key)
    local t, mt = check_arg(table, key);

    if mt == res_magic then
        return read_resource(t);
    end;

    return t;
end;

new_cat = function(table, key)
    local meta = getmetatable(table);
    local mt = {
        __newindex = newindex, __index = index,
        _magic = cat_magic, _fullname = meta._fullname .. '.' .. key, _state = 0
    };
    meta[key] = setmetatable({}, mt);
    return meta[key];
end;

local function register_resource(root, name, object)
    local meta = getmetatable(root);
    meta._state = 1;

    local head, tail = string.match(name, '^(%a+)%.?(.*)');
    local t = meta[head];
    if tail == '' then
        meta[head] = object;
        if getmetatable(object) == res_magic then
            write_resource(object, t);
        end;
        return t;
    end;

    if t == nil then
        t = new_cat(root, head);
    end;
    return register_resource(t, tail, object);
end;

local function dump_value(val)
    if type(val) == "string" then
        return string.format('%q', val);
    elseif type(val) == "number" then
        return string.format('%g', val);
    else
        error('Unsupported value type: ' .. type(val));
    end;
end;

local function dump_(root, fd)
    local meta = getmetatable(root);
    if not string.match(meta._fullname, "^[%a.]+$") then
        error("Someone has been messing with metatables.");
    end;

    for k, v in pairs(meta) do
        if type(k) == "string" and string.match(k, "^%a+$") then
            local mt = getmetatable(v);

            fd:write(meta._fullname, '.', k, ' = ');
            if mt ~= nil and mt._magic == cat_magic then
                fd:write('{}\n');
                dump(v);
                fd:write('\n');
            else
                if mt == res_magic then
                    v = read_resource(v);
                end;
                fd:write(dump_value(v), '\n');
            end;
        end;
    end;
end;

local function dump(root, file)
    local fd = io.open(file, 'w');
    dump_(root, fd);
    fd:close();
end;

local function make_root(name)
    local t = {
        __newindex = newindex, __index = index,
        _magic = cat_magic, _fullname = name, _state = 0
    };
    getfenv()[name] = setmetatable({}, t);
end;

return make_root, register_resource, dump;

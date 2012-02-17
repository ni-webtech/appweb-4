/*
   xcode.es -- Support functions for generating Xcode projects
        
   Exporting: xcode()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     
    
require ejs.unix
    
var out: Stream

const ARCH_VERSION = '1'
const OBJ_VERSION = 46

public function xcode(base: Path) {
    bit.ARCH_VERSION = ARCH_VERSION
    bit.OBJ_VERSION = OBJ_VERSION
    all(base)
    sources(base)
    let projects = []
    for each (target in bit.targets) {
        projBuild(projects, base, target)
    }
}

const all = uid()

function all(projects, base: Path) {
    output('    ' + all + '/* All */ = {
        isa = PBXAggregateTarget;
        buildConfigurationList = C90A1A0E0FD6DD14006A9E86 /* Build configuration list for PBXAggregateTarget "All" */;
        buildPhases = (
            /* MOB - replace with UID, for shell scripts */
        );
        dependencies = (')
    for each (target in bit.targets) {
        if (target.type != 'exe' && target.type != 'lib') {
            continue
        }
        target.uid ||= uid()
        output('        ' + target.uid + '/* ' + target.name + ' */,')
        output('        );
        name = All;
        productName = All;
        };')
    }
}

//  MOB - what about other aggregated targets: test, 
var fileID = {}

/* PBXBuildFile section */
function sources(base, target) {
    for each (file in target.files) {
        let obj = bit.targets[file]
        for each (src in obj.files) {
            let path = src.relativeTo(base)
            let id = fileID[src]
            output('    ' + id + ' = { isa = PBXBuildFile; fileRef = ' + id + ' /* ' + src + ' */; };')
        }
    }
    output('</ItemGroup>')
}

/* PBXFileReference section */
function files(base, target) {
    for each (file in target.files) {
        let obj = bit.targets[file]
        for each (src in obj.files) {
            let path = src.relativeTo(base)
            let id = fileID[src]
            //  MOB - some have SOURCE_ROOT or BUILT_PRODUCTS_DIR instead of <group>
            output('    ' + id + 
                ' = { isa = PBXBuildFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.c; ' +
                'name = ' + src + '; path = ' + path + '; sourceTree = "<group>"; };')
        }
        if (target.type == 'lib') {
            output('    ' + target.uid + 
                ' = { isa = PBXBuildFileReference; fileEncoding = 4; explicitFileType = compiled.mach-o.dylib; ' +
                'name = ' + src + '; includeInIndex = 0; path = ' + path + '; sourceTree = BUILT_PRODUCTS_DIR; };')
        } else if (target.type == 'exe') {
            output('    ' + target.uid + 
                ' = { isa = PBXBuildFileReference; fileEncoding = 4; lastKnownFileType = compiled.mach-o.executable; ' +
                explicitFileType = "compiled.mach-o.dylib";
                'name = ' + src + '; includeInIndex = 0; path = ' + path + '; sourceTree = BUILT_PRODUCTS_DIR; };')
        }
    }
}

function projBuild(projects: Array, base: Path, target) {
    if (target.built || !target.enable) {
        return
    }
    if (target.type != 'exe' && target.type != 'lib') {
        return
    }
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (dep && dep.enable && !dep.built) {
            projBuild(projects, base, dep)
        }
    }
    let path = base.join(target.name).joinExt('vcxproj').relative
    target.project = path
    projects.push(target)
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))
    projHeader(base, target)
    projConfig(base, target)
    projSources(base, target)
    projLink(base, target)
    projDeps(base, target)
    projFooter(base, target)
    out.close()
    target.built = true
}

function projHeader(base, target) {
    bit.SUBSYSTEM = (target.rule == 'gui') ? 'Windows' : 'Console'
    bit.INC = target.includes.map(function(path) wpath(path.relativeTo(base))).join(';')

    output('// !$*UTF8*$!
    {
    archiveVersion = 1;
    classes = {
    };
    objectVersion = 46;
    objects = {')
}

function projConfig(base, target) {
    bit.PTYPE = (target.type == 'exe') ? 'Application' : 'DynamicLibrary'
    bit.VTYPE = 'Win32'
    bit.GUID = target.guid = Cmd('uuidgen').response.toLower().trim()
    bit.CTOK = '$(Configuration)'
    bit.PTOK = '$(Platform)'
    bit.STOK = '$(SolutionDir)'
    bit.OTOK = '$(OutDir)'
    bit.UTOK = '$(UserRootDir)'
    bit.VTOK = '$(VCTargetsPath)'
    bit.NAME = target.name
    bit.OUTDIR = wpath(bit.dir.cfg.relativeTo(base))

    // <Import Project="product.props" />

    output('
<ItemGroup Label="ProjectConfigurations">
  <ProjectConfiguration Include="Debug|${VTYPE}">
    <Configuration>Debug</Configuration>
    <Platform>${VTYPE}</Platform>
  </ProjectConfiguration>
</ItemGroup>

<PropertyGroup Label="Globals">
  <ProjectGuid>{${GUID}}</ProjectGuid>
  <RootNamespace />
  <Keyword>${VTYPE}Proj</Keyword>
</PropertyGroup>

<Import Project="${VTOK}\Microsoft.Cpp.Default.props" />

<PropertyGroup Condition="\'${CTOK}|${PTOK}\'==\'Debug|${VTYPE}\'" Label="Configuration">
  <ConfigurationType>${PTYPE}</ConfigurationType>
  <CharacterSet>NotSet</CharacterSet>
</PropertyGroup>

<Import Project="${VTOK}\Microsoft.Cpp.props" />

<ImportGroup Label="ExtensionSettings">
</ImportGroup>

<PropertyGroup Label="UserMacros" />

<PropertyGroup>
  <_ProjectFileVersion>${PROJECT_FILE_VERSION}</_ProjectFileVersion>
  <OutDir Condition="\'${CTOK}|${PTOK}\'==\'Debug|${VTYPE}\'">${OUTDIR}\\bin\\</OutDir>
  <IntDir Condition="\'${CTOK}|${PTOK}\'==\'Debug|${VTYPE}\'">${OUTDIR}\\obj\\${NAME}\\</IntDir>
</PropertyGroup>
    ')
}

//  MOB - should emit headers for all source that depends on headers
function projSourceHeaders(base, target) {
    /*
    if (target.type == 'header') {
    output('<ItemGroup>')
        output('  <ClInclude Include="' + wpath(target.path) + '" />')
    }
    output('</ItemGroup>')
    */
}

//  MOB - TODO
function vsresources(base, target) {
    output('<ItemGroup>')
    for each (file in target.files) {
        output('  <ClCompile Include="' + wpath(file) + '" />')
    }
    output('</ItemGroup>')
}

function projLink(base, target) {
    if (target.type == 'lib') {
        let def = base.join(target.path.basename.toString().replace(/dll$/, 'def'))
        let newdef = Path(target.path.toString().replace(/dll$/, 'def'))
        if (newdef.exists) {
            cp(newdef, def)
        }
        if (def.exists) {
            bit.DEF = wpath(def.relativeTo(base))
            output('
    <ItemDefinitionGroup>
    <Link>
      <ModuleDefinitionFile>${DEF}</ModuleDefinitionFile>
    </Link>
    </ItemDefinitionGroup>')
        }
    }
    bit.LIBS = (target.libraries - bit.defaults.libraries).map(function(l) 'lib'+l+'.lib').join(';')
    output('<ItemDefinitionGroup>
<Link>
  <AdditionalDependencies>${LIBS};%(AdditionalDependencies)</AdditionalDependencies>
</Link>
</ItemDefinitionGroup>')
}

function projDeps(base, target) {
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (!dep) {
            if (bit.packs[dname]) {
                continue
            }
            throw 'Missing dependency ' + dname + ' for target ' + target.name
        }
        //  MOB - should do something with objects, headers
        if (dep.type != 'exe' && dep.type != 'lib') {
            continue
        }
        if (!dep.enable) {
            continue
        }
        if (!dep.guid) {
            throw 'Missing guid for ' + dname
        }
        bit.DEP = dname
        bit.GUID = dep.guid
        output('
<ItemGroup>
  <ProjectReference Include="${DEP}.vcxproj">
  <Project>${GUID}</Project>
  <ReferenceOutputAssembly>false</ReferenceOutputAssembly>
  </ProjectReference>
</ItemGroup>')
    }
}

function projFooter(base, target) {
    output('rootObject = C90A19DC0FD6DC5E006A9E86;\n}\n')
}

function output(line: String) {
    out.writeLine(line.expand(bit))
}

function uid() ("%08x%08x%08x" % [Date().ticks, Date().ticks, Date().ticks]).toUpper()

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2012. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

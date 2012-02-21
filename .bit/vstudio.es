/*
   vstudio.es -- Support functions for generating VS projects
        
   Exporting: vstudio()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     
    
require ejs.unix
    
var out: Stream

const TOOLS_VERSION = '4.0'
const PROJECT_FILE_VERSION = 10.0.30319.1
const SOL_VERSION = '11.00'
const XID = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'

public function vstudio(base: Path) {
    bit.TOOLS_VERSION = TOOLS_VERSION
    bit.PROJECT_FILE_VERSION = PROJECT_FILE_VERSION
    let projects = []
    for each (target in bit.targets) {
        projBuild(projects, base, target)
    }
    solBuild(projects, base)
}

function solBuild(projects, base: Path) {
    let path = base.joinExt('sln').relative
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))
    output('Microsoft Visual Studio Solution File, Format Version ' + SOL_VERSION)
    output('# Visual Studio 2010')

    for each (target in projects) {
        target.guid = target.guid.toUpper()
        output('Project("' + XID + '") = "' + target.name + '", "' + 
            wpath(base.basename.join(target.name).joinExt('vcxproj')) + 
            '", "{' + target.guid + '}"')
        for each (dname in target.depends) {
            let dep = bit.targets[dname]
            if (!dep || !dep.guid) {
                continue
            }
            dep.guid = dep.guid.toUpper()
            output('\tProjectSection(ProjectDependencies) = postProject')
            output('\t\t{' + dep.guid + '} = {' + dep.guid + '}')
            output('\tEndProjectSection')
        }
        output('EndProject')
    }
    output('
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution')

    for each (target in projects) {
		output('{' + target.guid + '}.Debug|Win32.ActiveCfg = Debug|Win32')
		output('{' + target.guid + '}.Debug|Win32.Build.0 = Debug|Win32')
    }
	output('EndGlobalSection

  GlobalSection(SolutionProperties) = preSolution
    HideSolutionNode = FALSE
  EndGlobalSection
EndGlobal')
    out.close()
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
    output('<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="${TOOLS_VERSION}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros" />
  <PropertyGroup />
  <ItemDefinitionGroup>
    <ClCompile>
      <WarningLevel>Level3</WarningLevel>
      <AdditionalIncludeDirectories>${INC};%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
    ')

    if (bit.settings.profile == 'debug') {
        output('  <PreprocessorDefinitions>WIN32;_DEBUG;_WINDOWS;DEBUG_IDE;_REENTRANT;_MT;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    <Optimization>Disabled</Optimization>
    <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
    <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>')
    } else {
        output('  <PreprocessorDefinitions>WIN32;_WINDOWS;_REENTRANT;_MT;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    <FavorSizeOrSpeed>Size</FavorSizeOrSpeed>
    <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
    <Optimization>MinSpace</Optimization>
    <IntrinsicFunctions>true</IntrinsicFunctions>
    <FunctionLevelLinking>true</FunctionLevelLinking>')
    }

    output('    </ClCompile>
    <Link>
      <AdditionalDependencies>ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
      <AdditionalLibraryDirectories>$(OutDir);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      <SubSystem>${SUBSYSTEM}</SubSystem>')

    if (bit.settings.profile == 'debug') {
      output('      <GenerateDebugInformation>true</GenerateDebugInformation>')
    } else {
      output('      <GenerateDebugInformation>false</GenerateDebugInformation>')
    }
    output('    </Link>
  </ItemDefinitionGroup>
  <ItemGroup />')
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

function projSources(base, target) {
    output('<ItemGroup>')
    for each (file in target.files) {
        let obj = bit.targets[file]
        for each (src in obj.files) {
            let path = src.relativeTo(base)
            output('  <ClCompile Include="' + wpath(path) + '" />')
        }
    }
    output('</ItemGroup>')
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

    let libs = []
    for each (libname in (target.libraries - bit.defaults.libraries)) {
        let path = bit.dir.lib.join('lib' + libname).joinExt(bit.ext.shlib)
        if (path.exists) {
            libs.push(path)
        } else {
            libs.push(Path(libname).joinExt(bit.ext.shlib))
        }
    }
    //  MOB - convert when target.libpaths exists
    bit.LIBS = libs.join(';')
    let libpaths = []
    for each (flag in target.linker) {
        if (flag.contains('-libpath:')) {
            libpaths.push(flag.replace('-libpath:', ''))
        }
    }
    bit.LIBPATHS = libpaths.join(';')
    //MOB bit.LIBS = (target.libraries - bit.defaults.libraries).map(function(l) 'lib' + l + '.lib').join(';')
    output('<ItemDefinitionGroup>
<Link>
  <AdditionalDependencies>${LIBS};%(AdditionalDependencies)</AdditionalDependencies>
  <AdditionalLibraryDirectories>$(OutDir);${LIBPATHS};%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
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
    output('\n<Import Project="${VTOK}\Microsoft.Cpp.targets" />')
    output('<ImportGroup Label="ExtensionTargets">\n</ImportGroup>\n\n</Project>')
}

function output(line: String) {
    out.writeLine(line.expand(bit))
}

function wpath(path: Path)
    Path(path.relative.toString().replace(/\//g, '\\'))

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

;
; install.iss -- Inno Setup 4 install configuration file for Embedthis Appweb
;
; Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
;

[Setup]
AppName=!!BLD_NAME!!
AppVerName=!!BLD_NAME!! !!BLD_VERSION!!-!!BLD_NUMBER!!
DefaultDirName={pf}\!!BLD_NAME!!
DefaultGroupName=!!BLD_NAME!!
UninstallDisplayIcon={app}/!!BLD_PRODUCT!!.exe
LicenseFile=!!ORIG_BLD_PREFIX!!/LICENSE.TXT

[Code]
var
	PortPage: TInputQueryWizardPage;
	SSLPortPage: TInputQueryWizardPage;
	WebDirPage: TInputDirWizardPage;


procedure InitializeWizard();
begin

	WebDirPage := CreateInputDirPage(wpSelectDir, 'Select Web Documents Directory', 'Where should web files be stored?',
		'Select the folder in which to store web documents, then click Next.', False, '');
	WebDirPage.Add('');
	WebDirPage.values[0] := ExpandConstant('{sd}') + '/appweb/web';

	PortPage := CreateInputQueryPage(wpSelectComponents, 'HTTP Port', 'Primary TCP/IP Listen Port for HTTP Connections',
		'Please specify the TCP/IP port on which Appweb should listen for HTTP requests.');
	PortPage.Add('HTTP Port:', False);
	PortPage.values[0] := '80';

	SSLPortPage := CreateInputQueryPage(PortPage.ID, 'SSL Port', 'TCP/IP Port for SSL Connections',
		'Please specify the TCP/IP port on which Appweb should listen for SSL requests.');
	SSLPortPage.Add('SSL Port:', False);
	SSLPortPage.values[0] := '443';

end;


procedure CurStepChanged(CurStep: TSetupStep);
var
  path: String;
  app: String;
  rc: Integer;
begin
  if CurStep = ssInstall then
  begin
   app := ExpandConstant('{app}');
   
   path := app + '/bin/appwebMonitor.exe';
   if FileExists(path) then
     Exec(path, '--stop', app, 0, ewWaitUntilTerminated, rc);

   path := app + '/bin/angel.exe';
   if FileExists(path) then
     Exec(path, '--stop appweb', app, 0, ewWaitUntilTerminated, rc);
     
  end;
end;


function NextButtonClick(CurPageID: Integer): Boolean;
begin
  if (CurPageID = 6) then
  begin
    WebDirPage.values[0] := ExpandConstant('{app}') + '/web';
  end;
  Result := true;
end;

function SaveSettings(junk: String): Boolean;
var
  app: String;
  path: String;
  settings: String;
begin
  app := ExpandConstant('{app}');
  CreateDir(app);
  path := app + '/install.log';
  settings := '{ port: ' + PortPage.Values[0] + ', ssl: ' + SSLPortPage.Values[0] + 
	', web: "' + WebDirPage.Values[0] + '", root: "' + ExpandConstant('{app}') + '", }' + #13#10;
  StringChangeEx(settings, '\', '/', True);
  Result := SaveStringToFile(path, settings, False);
end;


function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo,
  MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  S: String;
begin

  S := '';
  S := S + MemoDirInfo + NewLine;
  S := S + 'Web Documents Directory:' + NewLine + Space + WebDirPage.Values[0] + NewLine;
  S := S + MemoComponentsInfo + NewLine;
  S := S + MemoGroupInfo + NewLine;
  S := S + 'HTTP Port:' + NewLine + Space + PortPage.Values[0] + NewLine;
  S := S + 'SSL  Port:' + NewLine + Space + SSLPortPage.Values[0] + NewLine;
  S := S + NewLine + NewLine;

  SaveSettings('');

  Result := S;
end;


function GetWebDir(Param: String): String;
begin
  Result := WebDirPage.Values[0];
end;


function GetPort(Param: String): String;
begin
  Result := PortPage.Values[0];
end;


function IsPresent(const file: String): Boolean;
begin
  file := ExpandConstant(file);
  if FileExists(file) then begin
    Result := True;
  end else begin
    Result := False;
  end
end;


[Icons]
Name: "{group}\!!BLD_NAME!!Monitor"; Filename: "{app}/bin/!!BLD_PRODUCT!!Monitor.exe"; Components: bin
Name: "{group}\Documentation"; Filename: "http://127.0.0.1:{code:GetPort}/doc/index.html"; Components: dev
Name: "{group}\ReadMe"; Filename: "{app}/README.TXT"
;Name: "{group}\Manage"; Filename: "http://127.0.0.1:{code:GetPort}/index.html"; Components: bin

[Registry]
Root: HKLM; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "AppwebMonitor"; ValueData: "{app}\bin\appwebMonitor.exe"

[Types]
Name: "full"; Description: "Complete Installation"; 
Name: "binary"; Description: "Binary Installation"; 
Name: "development"; Description: "Development Documentation, Headers and Libraries"; 

[Components]
Name: "bin"; Description: "Binary Files"; Types: binary full;
Name: "dev"; Description: "Development Files"; Types: development full;

[Dirs]
Name: "{app}/logs"; Permissions: system-modify;
Name: "{app}/bin"

[UninstallDelete]
Type: files; Name: "{app}/appweb.conf";
Type: files; Name: "{app}/logs/access.log";
Type: files; Name: "{app}/logs/access.log.old";
Type: files; Name: "{app}/logs/error.log";
Type: files; Name: "{app}/logs/error.log.old";
Type: filesandordirs; Name: "{app}/*.obj";

[Run]
Filename: "{app}/bin/!!BLD_PRODUCT!!Monitor.exe"; Parameters: "--stop"; WorkingDir: "{app}/bin"; Check: IsPresent('{app}/bin/!!BLD_PRODUCT!!Monitor.exe'); StatusMsg: "Stopping the Appweb Monitor"; Flags: waituntilterminated;

Filename: "{app}/bin/angel.exe"; Parameters: "--uninstall appweb"; WorkingDir: "{app}"; Check: IsPresent('{app}/bin/angel.exe'); StatusMsg: "Stopping Appweb"; Flags: waituntilterminated; Components: bin

Filename: "{app}/bin/ajs.exe"; Parameters: "bin/patchConfig.es install.log"; WorkingDir: "{app}"; StatusMsg: "Updating Appweb configuration"; Flags: runhidden waituntilterminated; 

Filename: "{app}/bin/angel.exe"; Parameters: "--install appweb"; WorkingDir: "{app}"; StatusMsg: "Installing Appweb as a Windows Service"; Flags: waituntilterminated;

Filename: "{app}/bin/angel.exe"; Parameters: "--start appweb"; WorkingDir: "{app}"; StatusMsg: "Starting the Appweb Server"; Flags: waituntilterminated;

Filename: "{app}/bin/!!BLD_PRODUCT!!Monitor.exe"; Parameters: ""; WorkingDir: "{app}/bin"; StatusMsg: "Starting the Appweb Monitor"; Flags: waituntilidle;

Filename: "http://127.0.0.1:{code:GetPort}/index.html"; Description: "View the Documentation"; Flags: skipifsilent waituntilidle shellexec postinstall; Components: bin

[UninstallRun]
Filename: "{app}/bin/!!BLD_PRODUCT!!Monitor.exe"; Parameters: "--stop"; WorkingDir: "{app}"; StatusMsg: "Stopping the Appweb Monitor"; Flags: waituntilterminated;
Filename: "{app}/bin/angel.exe"; Parameters: "--uninstall appweb"; WorkingDir: "{app}"; Check: IsPresent('{app}/bin/angel.exe'); Components: bin

Filename: "{app}/bin/removeFiles.exe"; Parameters: "-r -s 5"; WorkingDir: "{app}"; Flags:

[Files]

program qtiplotlaz;

{$mode objfpc}{$H+}

uses
  {$IFDEF UNIX}
  cthreads,
  {$ENDIF}
  Interfaces, // LCL widgetset
  Forms,
  frmmain;

begin
  RequireDerivedFormResource := True;
  Application.Title := 'QtiPlot Lazarus Port';
  Application.Scaled := True;
  Application.Initialize;
  Application.CreateForm(TMainForm, MainForm);
  Application.Run;
end.

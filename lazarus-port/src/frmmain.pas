unit frmmain;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, Grids;

type
  TMainForm = class(TForm)
    btnSmoke: TButton;
    grdTable: TStringGrid;
    procedure btnSmokeClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  end;

var
  MainForm: TMainForm;

implementation

{$R *.lfm}

procedure TMainForm.FormCreate(Sender: TObject);
var
  r: Integer;
begin
  Caption := 'QtiPlot Lazarus Port - Spike 0';
  grdTable.ColCount := 3;
  grdTable.RowCount := 11;
  grdTable.FixedRows := 1;
  grdTable.Cells[0, 0] := 'i';
  grdTable.Cells[1, 0] := 'i^2';
  grdTable.Cells[2, 0] := 'ISO 8601';
  for r := 1 to 10 do
  begin
    grdTable.Cells[0, r] := IntToStr(r);
    grdTable.Cells[1, r] := IntToStr(r * r);
    grdTable.Cells[2, r] := FormatDateTime('yyyy-mm-dd', EncodeDate(2026, 10, 5) + r);
  end;
end;

procedure TMainForm.btnSmokeClick(Sender: TObject);
begin
  ShowMessage('Spike 0 OK: LCL gtk2 on ' + FormatDateTime('yyyy-mm-dd hh:nn', Now));
end;

end.

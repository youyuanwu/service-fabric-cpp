$ErrorActionPreference = "Stop";

.\scripts\echomain_ctl.ps1 -Action Add

start-sleep -seconds 20

.\scripts\echomain_ctl.ps1 -Action Resolve

.\scripts\echomain_ctl.ps1 -Action Echo -Mode Test

.\scripts\echomain_ctl.ps1 -Action Remove
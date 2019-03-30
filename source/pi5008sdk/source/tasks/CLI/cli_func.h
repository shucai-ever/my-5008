#ifdef SUPPORT_DEBUG_CLI
#ifndef __CLI_FUNC_H__
#define __CLI_FUNC_H__

void _mdelay(int ms);

int _test(int argc, const char **argv);

int _pireg(int argc, const char **argv);

int _pvirx_test(int argc, const char **argv);
int _pvirx_mode(int argc, const char **argv);
int _pvirx_utcsend(int argc, const char **argv);

int _pvitx_test(int argc, const char **argv);
int _pvitx_mode(int argc, const char **argv);

int _vidinsel(int argc, const char **argv);
int _svminsel(int argc, const char **argv);
int _recinsel(int argc, const char **argv);
int _recpath(int argc, const char **argv);
int _setQuadView(int argc, const char **argv);
int _caminsel(int argc, const char **argv);
int _capture(int argc, const char **argv);
int _capscl(int argc, const char **argv);
int _monitor_pvi(int argc, const char **argv);
int _adc(int argc, const char **argv);
int _genlock(int argc, const char **argv);
int _vpuctl(int argc, const char **argv);
int _vputest(int argc, const char **argv);
int _alivetask(int argc, const char **argv);
int _bgtest(int argc, const char **argv);
int _proc(int argc, const char **argv);
int _event(int argc, const char **argv);
int _fwdn(int argc, const char **argv);
int _remoconkey(int argc, const char **argv);
int _sdfs(int argc, const char **argv);
int _flashop(int argc, const char **argv);
int _ipctest(int argc, const char **argv);
int _gpio(int argc, const char **argv);
int _top(int argc, const char **argv);
int _dumpreg(int argc, const char **argv);
int _testOD(int argc, const char **argv);
int _viewgen(int argc, const char **argv);
int _reset(int argc, const char **argv);
int _nandtest(int argc, const char **argv);
int _ftl(int argc, const char **argv);
int _dramest(int argc, const char **argv);
int _compare(int argc, const char **argv);
int _spitest(int argc, const char **argv);
int _calib(int argc, const char **argv);
int _i2c_test(int argc, const char **argv);
int _cache_test(int argc, const char **argv);
int _isp_test(int argc, const char **argv);
int _viewmode(int argc, const char **argv);
int _wboot_test(int argc, const char **argv);
int _pd_test(int argc, const char **argv);
#ifdef BD_SLT
int _core1_test(int argc, const char **argv);
int _slt_test(int argc, const char **argv);
int _slt_test_manual(void);
int _slt_test_auto_mipi_rx(void);
int _slt_test_auto_pvi_rx(void);
#endif // BD_SLT
int _i2c_reg(int argc, const char **argv);
#endif // __CLI_FUNC_H__
#endif /* SUPPORT_DEBUG_CLI */

#include "obs-frontend-api/obs-frontend-api.h"

#include "window-basic-stats.hpp"
#include "window-basic-main.hpp"
#include "platform.hpp"
#include "obs-app.hpp"
#include "menu-button.hpp"

#include <QDesktopWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <string>

#define TIMER_INTERVAL 2000

static void setThemeID(QWidget *widget, const QString &themeID)
{
	if (widget->property("themeID").toString() != themeID) {
		widget->setProperty("themeID", themeID);

		/* force style sheet recalculation */
		QString qss = widget->styleSheet();
		widget->setStyleSheet("/* */");
		widget->setStyleSheet(qss);
	}
}

OBSBasicStats::OBSBasicStats(QWidget *parent, bool closeable)
	: QWidget             (parent),
	  cpu_info            (os_cpu_usage_info_start()),
	  timer               (this)
{
	QVBoxLayout *mainLayout = new QVBoxLayout();
	QGridLayout *topLayout0 = new QGridLayout();
	QGridLayout *topLayout1 = new QGridLayout();
	QHBoxLayout *topLayout = new QHBoxLayout();
	outputLayout = new QGridLayout();

	int row = 0;

	/* --------------------------------------------- */

	cpuUsage = new QLabel(this);
	cpuUsage->setToolTip(QTStr("Basic.Stats.CPUUsage.ToolTip"));
	cpuUsage->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vCPUStats"));

	cpuUsageTxt = new QLabel(QTStr("Basic.Stats.CPUUsage"), this);
	cpuUsageTxt->setToolTip(QTStr("Basic.Stats.CPUUsage.ToolTip"));
	cpuUsageTxt->setVisible(!cpuUsage->isHidden());

	hddSpace = new QLabel(this);
	hddSpace->setToolTip(QTStr(
			"Basic.Stats.HDDSpaceAvailable.ToolTip"));
	hddSpace->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vHDDStats"));

	hddSpaceTxt = new QLabel(QTStr("Basic.Stats.HDDSpaceAvailable"), this);
	hddSpaceTxt->setToolTip(QTStr(
			"Basic.Stats.HDDSpaceAvailable.ToolTip"));
	hddSpaceTxt->setVisible(!hddSpace->isHidden());

	memUsage = new QLabel(this);
	memUsage->setToolTip(QTStr("Basic.Stats.MemoryUsage.ToolTip"));
	memUsage->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vMemoryStats"));

	memUsageTxt = new QLabel(QTStr("Basic.Stats.MemoryUsage"), this);
	memUsageTxt->setToolTip(QTStr(
			"Basic.Stats.MemoryUsage.ToolTip"));
	memUsageTxt->setVisible(!memUsage->isHidden());

	topLayout0->addWidget(cpuUsageTxt, row, 0);
	topLayout0->addWidget(cpuUsage, row++, 1);

	topLayout0->addWidget(hddSpaceTxt, row, 0);
	topLayout0->addWidget(hddSpace, row++, 1);

	topLayout0->addWidget(memUsageTxt, row, 0);
	topLayout0->addWidget(memUsage, row++, 1);

	row = 0;

	/* --------------------------------------------- */

	fps = new QLabel(this);
	fps->setToolTip(QTStr("Basic.Stats.FPS.ToolTip"));
	fps->setVisible(!config_get_bool(GetGlobalConfig(), "BasicWindow",
			"vFPSStats"));

	fpsTxt = new QLabel(QTStr("Basic.Stats.FPS"), this);
	fpsTxt->setToolTip(QTStr("Basic.Stats.FPS.ToolTip"));
	fpsTxt->setVisible(!fps->isHidden());

	renderTime = new QLabel(this);
	renderTime->setToolTip(QTStr(
			"Basic.Stats.AverageTimeToRender.ToolTip"));
	renderTime->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vRenderTimeStats"));

	renderTimeTxt = new QLabel(QTStr(
			"Basic.Stats.AverageTimeToRender"), this);
	renderTimeTxt->setToolTip(QTStr(
			"Basic.Stats.AverageTimeToRender.ToolTip"));
	renderTimeTxt->setVisible(!renderTime->isHidden());

	missedFrames = new QLabel(this);
	missedFrames->setToolTip(QTStr(
			"Basic.Stats.MissedFrames.ToolTip"));
	missedFrames->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vMissedFramesStats"));

	missedFramesTxt = new QLabel(QTStr("Basic.Stats.MissedFrames"), this);
	missedFramesTxt->setToolTip(QTStr(
			"Basic.Stats.MissedFrames.ToolTip"));
	missedFramesTxt->setVisible(!missedFrames->isHidden());

	skippedFrames = new QLabel(this);
	skippedFrames->setToolTip(QTStr(
			"Basic.Stats.SkippedFrames.ToolTip"));
	skippedFrames->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vSkippedFramesStats"));

	skippedFramesTxt = new QLabel(QTStr(
			"Basic.Stats.SkippedFrames"), this);
	skippedFramesTxt->setToolTip(QTStr(
			"Basic.Stats.SkippedFrames.ToolTip"));
	skippedFramesTxt->setVisible(!skippedFrames->isHidden());

	topLayout1->addWidget(fpsTxt, row, 0);
	topLayout1->addWidget(fps, row++, 1);

	topLayout1->addWidget(renderTimeTxt, row, 0);
	topLayout1->addWidget(renderTime, row++, 1);

	topLayout1->addWidget(missedFramesTxt, row, 0);
	topLayout1->addWidget(missedFrames, row++, 1);

	topLayout1->addWidget(skippedFramesTxt, row, 0);
	topLayout1->addWidget(skippedFrames, row++, 1);

	/* --------------------------------------------- */

	int col = 0;

	/* --------------------------------------------- */

	AddOutputLabels(QTStr("Basic.Stats.Output.Stream"));
	AddOutputLabels(QTStr("Basic.Stats.Output.Recording"));

	/* --------------------------------------------- */

	nameTxt = new QLabel(QTStr("Basic.Stats.Output"), this);
	nameTxt->setToolTip(QTStr("Basic.Stats.Output.ToolTip"));
	nameTxt->setStyleSheet("font-weight: bold");
	nameTxt->setVisible(!config_get_bool(GetGlobalConfig(), "BasicWindow",
			"vNameStats"));
	outputLabels[0].name->setVisible(!nameTxt->isHidden());
	outputLabels[1].name->setVisible(!nameTxt->isHidden());

	statusTxt = new QLabel(QTStr("Basic.Stats.Status"), this);
	statusTxt->setToolTip(QTStr("Basic.Stats.Status.ToolTip"));
	statusTxt->setStyleSheet("font-weight: bold");
	statusTxt->setVisible(!config_get_bool(GetGlobalConfig(), "BasicWindow",
			"vStatusStats"));
	outputLabels[0].status->setVisible(!statusTxt->isHidden());
	outputLabels[1].status->setVisible(!statusTxt->isHidden());

	droppedFramesTxt = new QLabel(QTStr("Basic.Stats.DroppedFrames"), this);
	droppedFramesTxt->setToolTip(QTStr(
			"Basic.Stats.DroppedFrames.ToolTip"));
	droppedFramesTxt->setStyleSheet("font-weight: bold");
	droppedFramesTxt->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vDroppedFramesStats"));
	outputLabels[0].droppedFrames->setVisible(
			!droppedFramesTxt->isHidden());
	outputLabels[1].droppedFrames->setVisible(
			!droppedFramesTxt->isHidden());

	megabytesSentTxt = new QLabel(QTStr("Basic.Stats.MegabytesSent"), this);
	megabytesSentTxt->setToolTip(QTStr(
			"Basic.Stats.MegabytesSent.ToolTip"));
	megabytesSentTxt->setStyleSheet("font-weight: bold");
	megabytesSentTxt->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vMegabytesSentStats"));
	outputLabels[0].megabytesSent->setVisible(
			!megabytesSentTxt->isHidden());
	outputLabels[1].megabytesSent->setVisible(
			!megabytesSentTxt->isHidden());

	bitrateTxt = new QLabel(QTStr("Basic.Stats.Bitrate"), this);
	bitrateTxt->setToolTip(QTStr("Basic.Stats.Bitrate.ToolTip"));
	bitrateTxt->setStyleSheet("font-weight: bold");
	bitrateTxt->setVisible(!config_get_bool(GetGlobalConfig(),
			"BasicWindow", "vBitrateStats"));
	outputLabels[0].bitrate->setVisible(!bitrateTxt->isHidden());
	outputLabels[1].bitrate->setVisible(!bitrateTxt->isHidden());

	outputLayout->addWidget(nameTxt, 0, col++);
	outputLayout->addWidget(statusTxt, 0, col++);
	outputLayout->addWidget(droppedFramesTxt, 0, col++);
	outputLayout->addWidget(megabytesSentTxt, 0, col++);
	outputLayout->addWidget(bitrateTxt, 0, col++);

	QVBoxLayout *outputContainerLayout = new QVBoxLayout();
	outputContainerLayout->addLayout(outputLayout);
	outputContainerLayout->addStretch();

	QWidget *widget = new QWidget(this);
	widget->setLayout(outputContainerLayout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidget(widget);
	scrollArea->setWidgetResizable(true);
	scrollArea->setMinimumSize(1, 1);

	/* --------------------------------------------- */

	/* Menu to toggle visibility of stats items */

	QAction *vCPU = new QAction(QTStr("Basic.Stats.CPUUsage"), this);
	QAction	*vHDD = new QAction(QTStr("Basic.Stats.HDDSpaceAvailable"),
			this);
	QAction *vMemory = new QAction(QTStr("Basic.Stats.MemoryUsage"), this);
	QAction *vFPS = new QAction(QTStr("Basic.Stats.FPS"), this);
	QAction *vRenderTime = new QAction(QTStr(
			"Basic.Stats.AverageTimeToRender"), this);
	QAction *vMissedFrames = new QAction(QTStr("Basic.Stats.MissedFrames"),
			this);
	QAction *vSkippedFrames = new QAction(QTStr(
			"Basic.Stats.SkippedFrames"), this);

	vCPU->setCheckable(true);
	vHDD->setCheckable(true);
	vMemory->setCheckable(true);
	vFPS->setCheckable(true);
	vRenderTime->setCheckable(true);
	vMissedFrames->setCheckable(true);
	vSkippedFrames->setCheckable(true);

	vCPU->setChecked(!cpuUsageTxt->isHidden());
	vHDD->setChecked(!hddSpaceTxt->isHidden());
	vMemory->setChecked(!memUsageTxt->isHidden());
	vFPS->setChecked(!fpsTxt->isHidden());
	vRenderTime->setChecked(!renderTimeTxt->isHidden());
	vMissedFrames->setChecked(!missedFramesTxt->isHidden());
	vSkippedFrames->setChecked(!skippedFramesTxt->isHidden());

	connect(vCPU, &QAction::changed, this, &OBSBasicStats::toggleCPU);
	connect(vHDD, &QAction::changed, this, &OBSBasicStats::toggleHDD);
	connect(vMemory, &QAction::changed, this, &OBSBasicStats::toggleMemory);
	connect(vFPS, &QAction::changed, this, &OBSBasicStats::toggleFPS);
	connect(vRenderTime, &QAction::changed, this,
			&OBSBasicStats::toggleRenderTime);
	connect(vMissedFrames, &QAction::changed, this,
			&OBSBasicStats::toggleMissedFrames);
	connect(vSkippedFrames, &QAction::changed, this,
			&OBSBasicStats::toggleSkippedFrames);

	/* Additional menu entries */

	QAction *vName = new QAction(QTStr("Basic.Stats.Output"), this);
	QAction	*vStatus = new QAction(QTStr("Basic.Stats.Status"), this);
	QAction *vDroppedFrames = new QAction(QTStr(
			"Basic.Stats.DroppedFrames"), this);
	QAction *vMegabytesSent = new QAction(QTStr(
			"Basic.Stats.MegabytesSent"), this);
	QAction *vBitrate = new QAction(QTStr("Basic.Stats.Bitrate"), this);

	vName->setCheckable(true);
	vStatus->setCheckable(true);
	vDroppedFrames->setCheckable(true);
	vMegabytesSent->setCheckable(true);
	vBitrate->setCheckable(true);

	vName->setChecked(!nameTxt->isHidden());
	vStatus->setChecked(!statusTxt->isHidden());
	vDroppedFrames->setChecked(!droppedFramesTxt->isHidden());
	vMegabytesSent->setChecked(!megabytesSentTxt->isHidden());
	vBitrate->setChecked(!bitrateTxt->isHidden());

	connect(vName, &QAction::changed, this, &OBSBasicStats::toggleName);
	connect(vStatus, &QAction::changed, this, &OBSBasicStats::toggleStatus);
	connect(vDroppedFrames, &QAction::changed, this,
			&OBSBasicStats::toggleDroppedFrames);
	connect(vMegabytesSent, &QAction::changed, this,
			&OBSBasicStats::toggleMegabytesSent);
	connect(vBitrate, &QAction::changed, this,
			&OBSBasicStats::toggleBitrate);

	QPushButton *resetButton = new MenuButton();
	resetButton->setText(QTStr("Reset"));

	QMenu *popup = new QMenu(resetButton);

	popup->addAction(vCPU);
	popup->addAction(vHDD);
	popup->addAction(vMemory);
	popup->addSeparator();
	popup->addAction(vFPS);
	popup->addAction(vRenderTime);
	popup->addAction(vMissedFrames);
	popup->addAction(vSkippedFrames);
	popup->addSeparator();
	popup->addAction(vName);
	popup->addAction(vStatus);
	popup->addAction(vDroppedFrames);
	popup->addAction(vMegabytesSent);
	popup->addAction(vBitrate);

	resetButton->setMenu(popup);

	/* --------------------------------------------- */

	QPushButton *closeButton = nullptr;

	if (closeable)
		closeButton = new QPushButton(QTStr("Close"));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(resetButton);

	if (closeable)
		connect(closeButton, &QPushButton::clicked,
				[this] () {close();});
	connect(resetButton, &QAbstractButton::clicked, [this]() {Reset(); });

	/* --------------------------------------------- */

	QVBoxLayout *topLeftLayout = new QVBoxLayout();
	QVBoxLayout *topRightLayout = new QVBoxLayout();

	topLeftLayout->addLayout(topLayout0);
	topLeftLayout->addStretch();
	topRightLayout->addLayout(topLayout1);
	topRightLayout->addStretch();

	topLayout->addLayout(topLeftLayout);
	topLayout->addLayout(topRightLayout);

	mainLayout->addLayout(topLayout);
	mainLayout->addWidget(scrollArea);
	mainLayout->addLayout(buttonLayout);
	setLayout(mainLayout);

	/* --------------------------------------------- */

	delete shortcutFilter;
	shortcutFilter = CreateShortcutFilter();
	installEventFilter(shortcutFilter);

	resize(800, 280);

	setWindowTitle(QTStr("Basic.Stats"));
	setWindowIcon(QIcon::fromTheme("obs", QIcon(":/res/images/obs.png")));

	setWindowModality(Qt::NonModal);
	setAttribute(Qt::WA_DeleteOnClose, true);

	QObject::connect(&timer, &QTimer::timeout, this, &OBSBasicStats::Update);
	timer.setInterval(TIMER_INTERVAL);
	timer.start();
	Update();

	OBSBasic *main = reinterpret_cast<OBSBasic*>(App()->GetMainWindow());

	const char *geometry = config_get_string(main->Config(),
			"Stats", "geometry");
	if (geometry != NULL) {
		QByteArray byteArray = QByteArray::fromBase64(
				QByteArray(geometry));
		restoreGeometry(byteArray);

		QRect windowGeometry = normalGeometry();
		if (!WindowPositionValid(windowGeometry)) {
			QRect rect = App()->desktop()->geometry();
			setGeometry(QStyle::alignedRect(
						Qt::LeftToRight,
						Qt::AlignCenter,
						size(), rect));
		}
	}
}

void OBSBasicStats::toggleCPU()
{
	bool visible = cpuUsageTxt->isHidden();
	cpuUsageTxt->setVisible(visible);
	cpuUsage->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
		"vCPUStats", !visible);
}

void OBSBasicStats::toggleHDD()
{
	bool visible = hddSpace->isHidden();
	hddSpace->setVisible(visible);
	hddSpaceTxt->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vHDDStats", !visible);
}

void OBSBasicStats::toggleMemory()
{
	bool visible = memUsage->isHidden();
	memUsage->setVisible(visible);
	memUsageTxt->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vMemoryStats", !visible);
}

void OBSBasicStats::toggleFPS()
{
	bool visible = fps->isHidden();
	fps->setVisible(visible);
	fpsTxt->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vFPSStats", !visible);
}

void OBSBasicStats::toggleRenderTime()
{
	bool visible = renderTime->isHidden();
	renderTime->setVisible(visible);
	renderTimeTxt->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vRenderTimeStats", !visible);
}

void OBSBasicStats::toggleMissedFrames()
{
	bool visible = missedFrames->isHidden();
	missedFrames->setVisible(visible);
	missedFramesTxt->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vMissedFramesStats", !visible);
}

void OBSBasicStats::toggleSkippedFrames()
{
	bool visible = skippedFrames->isHidden();
	skippedFrames->setVisible(visible);
	skippedFramesTxt->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vSkippedFramesStats", !visible);
}

void OBSBasicStats::toggleName()
{
	bool visible = nameTxt->isHidden();
	nameTxt->setVisible(visible);
	outputLabels[0].name->setVisible(visible);
	outputLabels[1].name->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vNameStats", !visible);
}

void OBSBasicStats::toggleStatus()
{
	bool visible = statusTxt->isHidden();
	statusTxt->setVisible(visible);
	outputLabels[0].status->setVisible(visible);
	outputLabels[1].status->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vStatusStats", !visible);
}

void OBSBasicStats::toggleDroppedFrames()
{
	bool visible = droppedFramesTxt->isHidden();
	droppedFramesTxt->setVisible(visible);
	outputLabels[0].droppedFrames->setVisible(visible);
	outputLabels[1].droppedFrames->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vDroppedFramesStats", !visible);
}

void OBSBasicStats::toggleMegabytesSent()
{
	bool visible = megabytesSentTxt->isHidden();
	megabytesSentTxt->setVisible(visible);
	outputLabels[0].megabytesSent->setVisible(visible);
	outputLabels[1].megabytesSent->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vMegabytesSentStats", !visible);
}

void OBSBasicStats::toggleBitrate()
{
	bool visible = bitrateTxt->isHidden();
	bitrateTxt->setVisible(visible);
	outputLabels[0].bitrate->setVisible(visible);
	outputLabels[1].bitrate->setVisible(visible);
	config_set_bool(GetGlobalConfig(), "BasicWindow",
			"vBitrateStats", !visible);
}

void OBSBasicStats::closeEvent(QCloseEvent *event)
{
	OBSBasic *main = reinterpret_cast<OBSBasic*>(App()->GetMainWindow());
	if (isVisible()) {
		config_set_string(main->Config(),
				"Stats", "geometry",
				saveGeometry().toBase64().constData());
		config_save_safe(main->Config(), "tmp", nullptr);
	}

	QWidget::closeEvent(event);
}

OBSBasicStats::~OBSBasicStats()
{
	delete shortcutFilter;
	os_cpu_usage_info_destroy(cpu_info);
}

void OBSBasicStats::AddOutputLabels(QString name)
{
	OutputLabels ol;
	ol.name = new QLabel(name, this);
	ol.status = new QLabel(this);
	ol.droppedFrames = new QLabel(this);
	ol.megabytesSent = new QLabel(this);
	ol.bitrate = new QLabel(this);

	ol.name->setToolTip(QTStr("Basic.Stats.Output.ToolTip"));
	ol.status->setToolTip(QTStr("Basic.Stats.Status.ToolTip"));
	ol.droppedFrames->setToolTip(QTStr(
			"Basic.Stats.DroppedFrames.ToolTip"));
	ol.megabytesSent->setToolTip(QTStr(
			"Basic.Stats.MegabytesSent.ToolTip"));
	ol.bitrate->setToolTip(QTStr("Basic.Stats.Bitrate.ToolTip"));

	int newPointSize = ol.status->font().pointSize();
	newPointSize *= 13;
	newPointSize /= 10;
	QString qss =
		QString("font-size: %1pt").arg(QString::number(newPointSize));
	ol.status->setStyleSheet(qss);

	int col = 0;
	int row = outputLabels.size() + 1;
	outputLayout->addWidget(ol.name, row, col++);
	outputLayout->addWidget(ol.status, row, col++);
	outputLayout->addWidget(ol.droppedFrames, row, col++);
	outputLayout->addWidget(ol.megabytesSent, row, col++);
	outputLayout->addWidget(ol.bitrate, row, col++);
	outputLabels.push_back(ol);
}

static uint32_t first_encoded = 0xFFFFFFFF;
static uint32_t first_skipped = 0xFFFFFFFF;
static uint32_t first_rendered = 0xFFFFFFFF;
static uint32_t first_lagged = 0xFFFFFFFF;

void OBSBasicStats::InitializeValues()
{
	video_t *video = obs_get_video();
	first_encoded  = video_output_get_total_frames(video);
	first_skipped  = video_output_get_skipped_frames(video);
	first_rendered = obs_get_total_frames();
	first_lagged   = obs_get_lagged_frames();
}

void OBSBasicStats::Update()
{
	OBSBasic *main = reinterpret_cast<OBSBasic*>(App()->GetMainWindow());

	/* TODO: Un-hardcode */

	struct obs_video_info ovi = {};
	obs_get_video_info(&ovi);

	OBSOutput strOutput = obs_frontend_get_streaming_output();
	OBSOutput recOutput = obs_frontend_get_recording_output();
	obs_output_release(strOutput);
	obs_output_release(recOutput);

	if (!strOutput && !recOutput)
		return;

	/* ------------------------------------------- */
	/* general usage                               */

	double curFPS = obs_get_active_fps();
	double obsFPS = (double)ovi.fps_num / (double)ovi.fps_den;

	QString str = QString::number(curFPS, 'f', 2);
	fps->setText(str);

	if (curFPS < (obsFPS * 0.8))
		setThemeID(fps, "error");
	else if (curFPS < (obsFPS * 0.95))
		setThemeID(fps, "warning");
	else
		setThemeID(fps, "");

	/* ------------------ */

	double usage = os_cpu_usage_info_query(cpu_info);
	str = QString::number(usage, 'g', 2) + QStringLiteral("%");
	cpuUsage->setText(str);

	/* ------------------ */

	const char *mode = config_get_string(main->Config(), "Output", "Mode");
	const char *path = strcmp(mode, "Advanced") ?
		config_get_string(main->Config(), "SimpleOutput", "FilePath") :
		config_get_string(main->Config(), "AdvOut", "RecFilePath");

#define MBYTE (1024ULL * 1024ULL)
#define GBYTE (1024ULL * 1024ULL * 1024ULL)
#define TBYTE (1024ULL * 1024ULL * 1024ULL * 1024ULL)
	uint64_t num_bytes = os_get_free_disk_space(path);
	QString abrv = QStringLiteral(" MB");
	long double num;

	num = (long double)num_bytes / (1024.0l * 1024.0l);
	if (num_bytes > TBYTE) {
		num /= 1024.0l * 1024.0l;
		abrv = QStringLiteral(" TB");
	} else if (num_bytes > GBYTE) {
		num /= 1024.0l;
		abrv = QStringLiteral(" GB");
	}

	str = QString::number(num, 'f', 1) + abrv;
	hddSpace->setText(str);

	if (num_bytes < GBYTE)
		setThemeID(hddSpace, "error");
	else if (num_bytes < (5 * GBYTE))
		setThemeID(hddSpace, "warning");
	else
		setThemeID(hddSpace, "");

	/* ------------------ */

	num = (long double)os_get_proc_resident_size() / (1024.0l * 1024.0l);

	str = QString::number(num, 'f', 1) + QStringLiteral(" MB");
	memUsage->setText(str);

	/* ------------------ */

	num = (long double)obs_get_average_frame_time_ns() / 1000000.0l;

	str = QString::number(num, 'f', 1) + QStringLiteral(" ms");
	renderTime->setText(str);

	long double fpsFrameTime =
		(long double)ovi.fps_den * 1000.0l / (long double)ovi.fps_num;

	if (num > fpsFrameTime)
		setThemeID(renderTime, "error");
	else if (num > fpsFrameTime * 0.75l)
		setThemeID(renderTime, "warning");
	else
		setThemeID(renderTime, "");

	/* ------------------ */

	video_t *video = obs_get_video();
	uint32_t total_encoded = video_output_get_total_frames(video);
	uint32_t total_skipped = video_output_get_skipped_frames(video);

	if (total_encoded < first_encoded || total_skipped < first_skipped) {
		first_encoded = total_encoded;
		first_skipped = total_skipped;
	}
	total_encoded -= first_encoded;
	total_skipped -= first_skipped;

	num = total_encoded
		? (long double)total_skipped / (long double)total_encoded
		: 0.0l;
	num *= 100.0l;

	str = QString("%1 / %2 (%3%)").arg(
			QString::number(total_skipped),
			QString::number(total_encoded),
			QString::number(num, 'f', 1));
	skippedFrames->setText(str);

	if (num > 5.0l)
		setThemeID(skippedFrames, "error");
	else if (num > 1.0l)
		setThemeID(skippedFrames, "warning");
	else
		setThemeID(skippedFrames, "");

	/* ------------------ */

	uint32_t total_rendered = obs_get_total_frames();
	uint32_t total_lagged   = obs_get_lagged_frames();

	if (total_rendered < first_rendered || total_lagged < first_lagged) {
		first_rendered = total_rendered;
		first_lagged   = total_lagged;
	}
	total_rendered -= first_rendered;
	total_lagged   -= first_lagged;

	num = total_rendered
		? (long double)total_lagged / (long double)total_rendered
		: 0.0l;
	num *= 100.0l;

	str = QString("%1 / %2 (%3%)").arg(
			QString::number(total_lagged),
			QString::number(total_rendered),
			QString::number(num, 'f', 1));
	missedFrames->setText(str);

	if (num > 5.0l)
		setThemeID(missedFrames, "error");
	else if (num > 1.0l)
		setThemeID(missedFrames, "warning");
	else
		setThemeID(missedFrames, "");

	/* ------------------------------------------- */
	/* recording/streaming stats                   */

	outputLabels[0].Update(strOutput, false);
	outputLabels[1].Update(recOutput, true);
}

void OBSBasicStats::Reset()
{
	timer.start();

	first_encoded  = 0xFFFFFFFF;
	first_skipped  = 0xFFFFFFFF;
	first_rendered = 0xFFFFFFFF;
	first_lagged   = 0xFFFFFFFF;

	OBSOutput strOutput = obs_frontend_get_streaming_output();
	OBSOutput recOutput = obs_frontend_get_recording_output();
	obs_output_release(strOutput);
	obs_output_release(recOutput);

	outputLabels[0].Reset(strOutput);
	outputLabels[1].Reset(recOutput);
	Update();
}

void OBSBasicStats::OutputLabels::Update(obs_output_t *output, bool rec)
{
	uint64_t totalBytes = output ? obs_output_get_total_bytes(output) : 0;
	uint64_t curTime = os_gettime_ns();
	uint64_t bytesSent = totalBytes;

	if (bytesSent < lastBytesSent)
		bytesSent = 0;
	if (bytesSent == 0)
		lastBytesSent = 0;

	uint64_t bitsBetween = (bytesSent - lastBytesSent) * 8;
	long double timePassed = (long double)(curTime - lastBytesSentTime) /
		1000000000.0l;
	long double kbps = (long double)bitsBetween /
		timePassed / 1000.0l;

	if (timePassed < 0.01l)
		kbps = 0.0l;

	QString str = QTStr("Basic.Stats.Status.Inactive");
	QString themeID;
	bool active = output ? obs_output_active(output) : false;
	if (rec) {
		if (active)
			str = QTStr("Basic.Stats.Status.Recording");
	} else {
		if (active) {
			bool reconnecting = output
				? obs_output_reconnecting(output)
				: false;

			if (reconnecting) {
				str = QTStr("Basic.Stats.Status.Reconnecting");
				themeID = "error";
			} else {
				str = QTStr("Basic.Stats.Status.Live");
				themeID = "good";
			}
		}
	}

	status->setText(str);
	setThemeID(status, themeID);

	long double num = (long double)totalBytes / (1024.0l * 1024.0l);

	megabytesSent->setText(
			QString("%1 MB").arg(QString::number(num, 'f', 1)));
	bitrate->setText(
			QString("%1 kb/s").arg(QString::number(kbps, 'f', 0)));

	if (!rec) {
		int total = output ? obs_output_get_total_frames(output) : 0;
		int dropped = output ? obs_output_get_frames_dropped(output) : 0;

		if (total < first_total || dropped < first_dropped) {
			first_total   = 0;
			first_dropped = 0;
		}

		total   -= first_total;
		dropped -= first_dropped;

		num = total
			? (long double)dropped / (long double)total * 100.0l
			: 0.0l;

		str = QString("%1 / %2 (%3%)").arg(
				QString::number(dropped),
				QString::number(total),
				QString::number(num, 'f', 1));
		droppedFrames->setText(str);

		if (num > 5.0l)
			setThemeID(droppedFrames, "error");
		else if (num > 1.0l)
			setThemeID(droppedFrames, "warning");
		else
			setThemeID(droppedFrames, "");
	}

	lastBytesSent     = bytesSent;
	lastBytesSentTime = curTime;
}

void OBSBasicStats::OutputLabels::Reset(obs_output_t *output)
{
	if (!output)
		return;

	first_total   = obs_output_get_total_frames(output);
	first_dropped = obs_output_get_frames_dropped(output);
}

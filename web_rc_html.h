// 网页遥控器客户端界面

#if WEB_RC_ENABLED

const char webRCIndexHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0,maximum-scale=1.0,user-scalable=no">
<title>琛光无人机遥控器--Ciallo～(∠・ω< )⌒★版</title>
<style>
/*======== 响应式变量 ========*/
:root{
  --js-size:clamp(150px,min(50vw,62vh),420px);
  --knob-size:calc(var(--js-size)*0.25);
  --pad:clamp(6px,1.5vmin,15px);
  --gap:clamp(6px,1.5vmin,15px);
}
/*======== 通用样式 ========*/
*{margin:0;padding:0;box-sizing:border-box;-webkit-tap-highlight-color:transparent;touch-action:none;user-select:none}
body{font-family:'Roboto Mono',Arial,"Microsoft YaHei",sans-serif;background:#3c3c3c;color:#fff;overflow:hidden;height:100vh;height:100dvh;width:100vw}
.container{width:100%;height:100%;display:flex;flex-direction:column;padding:var(--pad);gap:var(--gap);max-width:1200px;margin:0 auto;overflow:hidden}

/*======== 顶部状态栏 ========*/
.header {
  text-align: center;
  padding: 6px 10px;
  background: rgba(30,30,30,.9);
  border-radius: 12px;
  border: 2px solid rgba(150,150,150,.3);
  box-shadow: 0 4px 16px rgba(0,0,0,.4);
  backdrop-filter: blur(10px)
}

.header h1 {
  font-size: 1.1rem;
  color: #ffffff;
  margin-bottom: 4px;
}

.status-bar {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: clamp(4px,1.2vmin,10px);
  flex-wrap: nowrap;
  overflow: hidden;
  margin-top: 4px;
}

.status-item {
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 2px 5px;
  background: rgba(0,0,0,.3);
  border-radius: 6px;
  border: 1px solid rgba(255,255,255,.1);
  font-size: clamp(0.58rem,1.8vmin,0.7rem);
  white-space: nowrap;
  flex-shrink: 1;
  min-width: 0;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  display: inline-block;
}

.status-dot.connected { background: #0f8; box-shadow: 0 0 8px #0f8; }
.status-dot.disconnected { background: #f33; box-shadow: 0 0 8px #f33; }
.status-dot.warning { background: #ff9; box-shadow: 0 0 8px #ff9; }

/*======== 内容区 ========*/
.content{display:flex;flex:1;gap:var(--gap);overflow:hidden;min-height:0}
.joystick-container{flex:1;display:flex;flex-direction:column;justify-content:center;align-items:center;background:rgba(0,0,0,.4);border-radius:20px;padding:clamp(8px,1.5vmin,15px);border:2px solid rgba(255,255,255,0.15);box-shadow:inset 0 0 30px rgba(0,0,0,.5)}
.joystick-title{font-size:clamp(0.75rem,2.2vmin,1.1rem);color:#cccccc}
.joystick-wrapper{width:100%;height:var(--js-size);display:flex;justify-content:center;align-items:center;position:relative;margin-top:clamp(4px,2vh,20px);}
.joystick{width:var(--js-size);height:var(--js-size);background:radial-gradient(circle at 30% 30%,rgba(255,255,255,.1),rgba(0,0,0,.3));border-radius:50%;position:relative;border:2px solid rgba(255,255,255,0.2);box-shadow:inset 0 0 20px rgba(0,0,0,.5),0 8px 25px rgba(0,0,0,.5);overflow:hidden}
.joystick::before{content:'';position:absolute;top:50%;left:50%;width:2px;height:100%;background:linear-gradient(to bottom,transparent,rgba(255,255,255,.15),transparent);transform:translate(-50%,-50%)}
.joystick::after{content:'';position:absolute;top:50%;left:50%;width:100%;height:2px;background:linear-gradient(to right,transparent,rgba(255,255,255,.15),transparent);transform:translate(-50%,-50%)}
.joystick-knob{width:var(--knob-size);height:var(--knob-size);background:radial-gradient(circle at 30% 30%,#fff,#cccccc);border-radius:50%;position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);border:2px solid rgba(255,255,255,.7);box-shadow:0 4px 15px rgba(0,0,0,.5),inset 0 0 10px rgba(255,255,255,.5);cursor:move;transition:transform .1s ease-out;z-index:10}

/*======== 按钮区 ========*/
.buttons-container{flex:.55;display:flex;flex-direction:column;gap:10px;padding:12px;background:rgba(0,0,0,.4);border-radius:20px;border:2px solid rgba(150,150,150,.3);box-shadow:inset 0 0 20px rgba(0,0,0,.5)}
.buttons-grid{display:grid;grid-template-columns:repeat(6,1fr);grid-template-rows:0.5fr 0.5fr 1fr;gap:6px;flex:1}
.buttons-grid>button{grid-column:span 2}
.buttons-grid>button:nth-child(7),.buttons-grid>button:nth-child(8){grid-column:span 3}
.button{background:linear-gradient(145deg,#484848,#383838);border:none;border-radius:10px;color:#fff;font-size:.85rem;font-weight:bold;display:flex;flex-direction:column;justify-content:center;align-items:center;text-align:center;padding:10px 5px;cursor:pointer;transition:all .15s cubic-bezier(.4,0,.2,1);box-shadow:0 3px 10px rgba(0,0,0,.3),inset 0 1px 0 rgba(255,255,255,.1);position:relative}
.button:hover{background:linear-gradient(145deg,#565656,#464646);transform:translateY(-1px)}
.button.active{background:linear-gradient(145deg,#1a73e8,#0d47a1);box-shadow:0 0 15px rgba(26,115,232,.6),inset 0 1px 0 rgba(255,255,255,.2);transform:scale(.95)}
.button:active{transform:scale(.92)}
.button-icon{font-size:1.1rem;margin-bottom:4px}
.button.compact{padding:2px 2px;font-size:.66rem;border-radius:8px;white-space:nowrap;letter-spacing:-0.02em}
.button.compact .button-icon{font-size:.8rem;margin-bottom:0;line-height:1}

/*======== 调试控制台 ========*/
.console-panel{background:rgba(10,10,10,.95);border-radius:12px;border:1px solid rgba(100,100,100,.4);padding:10px;flex-shrink:0;max-height:min(200px,35vh);display:flex;flex-direction:column;gap:6px}
.console-output{flex:1;overflow-y:auto;font-family:'Courier New',monospace;font-size:0.7rem;color:#00ff88;min-height:80px;max-height:130px;word-break:break-all;-webkit-overflow-scrolling:touch}
.console-output div{padding:1px 0;border-bottom:1px solid rgba(255,255,255,.03)}

/*======== 动画 ========*/
@keyframes pulse{0%{box-shadow:0 0 0 0 rgba(67,97,238,.7)}70%{box-shadow:0 0 0 12px rgba(67,97,238,0)}100%{box-shadow:0 0 0 0 rgba(67,97,238,0)}}
.joystick.active{animation:pulse 1.5s infinite}
@keyframes fadeIn{from{opacity:0;transform:translateY(20px)}to{opacity:1;transform:translateY(0)}}
.container>*{animation:fadeIn .5s ease-out}

/*======== 竖屏自适应 ========*/
@media (orientation:portrait){
  :root{--js-size:clamp(120px,40vw,240px);--knob-size:calc(var(--js-size)*0.25)}
  .content{
    display:grid;
    grid-template-columns:repeat(2,minmax(0,1fr));
    grid-template-rows:auto auto;
    height:auto;
    flex:none;
    overflow:visible;
  }
  .buttons-container{grid-column:1/3;grid-row:1}
  .content>.joystick-container:first-child{grid-column:1;grid-row:2;min-width:0;overflow:hidden}
  .content>.joystick-container:last-child{grid-column:2;grid-row:2;min-width:0;overflow:hidden}
  .header h1{font-size:clamp(0.85rem,3.5vw,1.1rem)}
  .status-bar{gap:5px;flex-wrap:wrap;justify-content:center}
  .status-item{font-size:clamp(0.6rem,2.5vw,0.7rem);padding:2px 5px}
}

/*======== 小屏横屏自适应（高度≤420px）========*/
@media (max-height:420px) and (orientation:landscape){
  :root{--js-size:clamp(120px,min(42vw,44vh),240px);--knob-size:calc(var(--js-size)*0.25)}
  .joystick-title{font-size:0.72rem}
  .header h1{font-size:0.82rem}
  .header{padding:3px 8px}
  .header h1{margin-bottom:2px}
  .status-bar{margin-top:2px;gap:4px}
  .status-item{font-size:0.58rem;padding:2px 4px}
  .button{font-size:0.68rem;padding:5px 3px}
  .button-icon{font-size:0.88rem;margin-bottom:2px}
}
/*======== 网页看板娘 ========*/
.mascot-wrap{position:fixed;right:0;bottom:0;display:flex;justify-content:flex-end;align-items:flex-end;pointer-events:none;z-index:5}
.mascot{height:clamp(120px,26vh,280px);width:auto;max-width:42vw;object-fit:contain;pointer-events:auto;cursor:pointer;opacity:.95;filter:drop-shadow(0 0 10px rgba(0,170,255,.25));animation:mascotFloat 4s ease-in-out infinite}
@keyframes mascotFloat{0%,100%{transform:translateY(0)}50%{transform:translateY(-6px)}}
@media (max-height:520px){.mascot{height:clamp(90px,18vh,160px)}}

/*======== 调试窗看板娘 ========*/
.console-panel{position:relative;overflow:visible}
.console-panel>div{position:relative;z-index:1}
.mascot-debug{display:none;position:absolute;right:18px;top:100%;margin-top:-10px;height:clamp(88px,14vh,130px);width:auto;pointer-events:auto;cursor:pointer;opacity:.97;z-index:2;filter:drop-shadow(0 0 10px rgba(0,170,255,.2))}
body.console-open .console-panel{margin-bottom:calc(clamp(88px,14vh,130px) - 6px)}
body.console-open .mascot-debug{display:block}
body.console-open .mascot-wrap{display:none}

/*======== 配平弹窗 ========*/
.trim-modal{position:fixed;inset:0;background:rgba(0,0,0,.55);display:none;align-items:center;justify-content:center;z-index:40}
.trim-modal.show{display:flex}
.trim-box{background:#2b2b2b;border:1px solid #555;border-radius:14px;padding:16px 18px;width:min(92vw,360px);
  box-shadow:0 10px 30px rgba(0,0,0,.5);color:#fff}
.trim-title{font-size:15px;font-weight:700;text-align:center;margin-bottom:12px}
.trim-row{display:flex;align-items:center;gap:6px;margin:8px 0;font-size:12px}
.trim-row>span{flex:1}
.trim-row input{width:78px;background:#1b1b1b;border:1px solid #555;border-radius:6px;color:#0f8;
  padding:4px 6px;font-size:12px;text-align:center}
.trim-row button{width:26px;height:26px;border-radius:6px;border:none;background:#3a3a3a;color:#fff;font-size:14px;cursor:pointer}
.trim-actions{display:flex;gap:8px;justify-content:flex-end;margin-top:14px}
.trim-actions button{padding:6px 14px;border-radius:8px;border:none;font-size:12px;cursor:pointer}
#trim-cancel{background:#444;color:#ddd}
#trim-apply{background:#1a73e8;color:#fff}

/*======== 看板娘对话框 ========*/
.mascot-bubble{position:absolute;right:100%;bottom:45%;margin-right:10px;z-index:1;background:#fff;color:#222;
  padding:6px 10px;border-radius:11px;box-shadow:0 5px 14px rgba(0,0,0,.3);font-size:12px;font-weight:700;
  letter-spacing:.3px;white-space:nowrap;opacity:0;transform:translateX(6px);
  transition:opacity .18s ease,transform .18s ease;pointer-events:none}
.mascot-bubble.show{opacity:1;transform:translateX(0)}
.mascot-bubble::after{content:'';position:absolute;right:-7px;bottom:10px;
  border-top:6px solid transparent;border-bottom:6px solid transparent;border-left:8px solid #fff}
.mascot-bubble.in-console{right:10px;bottom:10px;margin-right:0}
.mascot-bubble.in-console::after{right:-7px;bottom:10px}

/*======== 业余模式：左油门 + 右配平键 ========*/
body.amateur #joystick-left{width:110px;height:var(--js-size);border-radius:55px}
body.amateur #joystick-left .joystick-knob{width:calc(var(--js-size)*0.18);height:calc(var(--js-size)*0.18)}
body.amateur #joystick-left::after{display:none}
.trim-pad{display:none;width:min(100%,var(--js-size));height:min(100%,var(--js-size));
  grid-template-columns:1fr 1.15fr 1fr;grid-template-rows:1fr 1.15fr 1fr;gap:clamp(4px,1vmin,8px)}
body.amateur .trim-pad{display:grid}
body.amateur #joystick-right{display:none}
.trim-pad button{border:none;border-radius:12px;background:#3a3a3a;color:#fff;font-weight:700;
  font-size:clamp(0.85rem,2.6vmin,1.25rem);cursor:pointer;box-shadow:0 3px 8px rgba(0,0,0,.35);
  display:flex;flex-direction:column;align-items:center;justify-content:center;line-height:1.15}
.trim-pad button small{font-size:.62em;font-weight:500;opacity:.8}
.trim-pad button.held,.trim-pad button:active{background:#1a73e8}
.trim-center{display:flex;flex-direction:column;align-items:center;justify-content:center;text-align:center;
  font-size:clamp(0.55rem,1.7vmin,0.75rem);color:#0f8;line-height:1.35;background:rgba(0,0,0,.35);border-radius:12px}

/*======== 版权页脚 ========*/
.footer{text-align:center;font-size:0.5rem;color:rgba(255,255,255,.25);padding:0;flex-shrink:0;line-height:0.8;}
.footer a{color:rgba(255,255,255,.3);text-decoration:none}
.footer a:hover{color:rgba(255,255,255,.55)}
</style>
</head>
<body>
<div class="container">
  <!-- 顶部状态栏 -->
  <div class="header">
    <h1>琛光无人机遥控器--Ciallo～(∠・ω&lt; )⌒★版</h1>
    <div class="status-bar">
      <div class="status-item"><span class="status-dot" id="status-dot"></span><span id="connection-text">连接中...</span></div>
      <div class="status-item" id="armed-status-item" style="background:rgba(255,51,51,0.15)"><span id="armed-status" style="color:#ff6666">已上锁</span></div>
      <div class="status-item"><span>飞行模式</span><span id="flight-mode">自稳</span></div>
      <div class="status-item"><span>电池电压</span><span id="battery">-</span></div>
      <div class="status-item"><span>遥控延迟</span><span id="latency">-</span></div>
      <div class="status-item"><span>丢包率</span><span id="packet-loss">0%</span></div>
      <div class="status-item"><span>油门:</span><span id="left-y">0</span><span>%</span></div>
      <div class="status-item"><span>偏航:</span><span id="left-x">0</span></div>
      <div class="status-item"><span>横滚:</span><span id="right-x">0</span></div>
      <div class="status-item"><span>俯仰:</span><span id="right-y">0</span></div>
    </div>
  </div>

  <div class="content">
    <!-- 左摇杆 -->
    <div class="joystick-container">
      <div class="joystick-title">左摇杆 (油门/偏航)</div>
      <div class="joystick-wrapper">
        <div class="joystick" id="joystick-left"><div class="joystick-knob" id="knob-left"></div></div>
      </div>
    </div>

    <!-- 按钮区 -->
    <div class="buttons-container">
      <div class="buttons-grid" id="buttons-container"></div>
    </div>

    <!-- 右摇杆；起步模式下换成配平方向键 -->
    <div class="joystick-container">
      <div class="joystick-title">右摇杆 (俯仰/横滚)</div>
      <div class="joystick-wrapper">
        <div class="joystick" id="joystick-right"><div class="joystick-knob" id="knob-right"></div></div>
        <div class="trim-pad" id="trim-pad">
          <span></span>
          <button type="button" data-droll="0" data-dpitch="1">上<small>俯仰+</small></button>
          <span></span>
          <button type="button" data-droll="-1" data-dpitch="0">左<small>横滚−</small></button>
          <div class="trim-center" id="trim-readout">横滚 +0.00°<br>俯仰 +0.00°</div>
          <button type="button" data-droll="1" data-dpitch="0">右<small>横滚+</small></button>
          <span></span>
          <button type="button" data-droll="0" data-dpitch="-1">下<small>俯仰−</small></button>
          <span></span>
        </div>
      </div>
    </div>
  </div>

  <!-- 调试控制台面板（默认隐藏，点击调试按钮打开） -->
  <div id="console-panel" class="console-panel" style="display:none">
    <img class="mascot-debug" src="/char2.webp" alt="">
    <div id="console-output" class="console-output"></div>
    <div style="display:flex;gap:6px;touch-action:auto">
      <input id="console-input" placeholder="输入命令 (ps/imu/rc/arm/disarm/help)..."
        style="flex:1;background:rgba(0,0,0,.6);border:1px solid rgba(100,100,100,.5);border-radius:6px;color:#0f8;padding:4px 8px;font-size:0.7rem;font-family:'Courier New',monospace;touch-action:auto">
      <button onclick="sendConsoleCmd()" style="background:#1a73e8;border:none;border-radius:6px;color:#fff;padding:4px 10px;font-size:0.7rem;cursor:pointer;touch-action:auto">发送</button>
    </div>
  </div>
  <!-- 看板娘 -->
  <div class="mascot-wrap"><img class="mascot" src="/char.webp" alt=""></div>
  <!-- 版权页脚 -->
  <div class="footer"><a href="https://oshwhub.com/songge8/project_qqqyfdkm" target="_blank">琛光无人机开源项目</a></div>
</div>

<script>
/*======================== 全局变量 ========================*/
let lastAnimationTime = 0;
let connectionOk = false;
let packetStats = { sent: 0, lost: 0 };
let latencyHistory = new Array(10).fill(0);
let latencyIndex = 0;
const SEND_INTERVAL = 50;  // ~20Hz 摇杆检测频率
const FORCE_SEND_INTERVAL = 200; // 静止时强制重发间隔（ms），保持飞控数据新鲜

const touches = new Map();
let leftStick  = {x:0, y:0, rawX:0, rawY:-100};
let rightStick = {x:0, y:0, rawX:0, rawY:0};

let lastSentValues = { throttle:0, roll:0, pitch:0, yaw:0 };
let lastForceSentTime = 0; // 上次强制重发的时间戳（performance.now()）
let currentValues  = { throttle:0, roll:0, pitch:0, yaw:0 };
let amateurMode = false;   // false=专业模式（双摇杆），true=起步模式（油门+配平键）
const TRIM_STEP = 0.005;   // rad/次，约 0.3°，与 CTL_TRIM_* 建议步长一致
const TRIM_REPEAT_MS = 160;
let trimRoll = 0, trimPitch = 0;
let trimBusy = false;
let trimPending = {roll:0, pitch:0};
let trimHold = null;
let isArmed = false;       // 当前解锁状态（由飞控响应同步）
const MIN_CHANGE_THRESHOLD = 0.5;

// 固定参数常量（替代前端参数调节面板，与后端 CONFIG_ 对应）
const DEADZONE = 3;   // 死区（已移至后端 stickDeadzone 统一处理，前端不再使用）
const EXPO    = 40;   // 指数曲线 40%
let consecutiveFails = 0; // 连续失败计数，>=3 才判定断连
let currentFlightMode = 2; // 当前飞行模式编号（与后端同步：2=自稳）

let buttonStates     = new Array(16).fill(false);
let lastButtonStates = new Array(16).fill(false);

let consolePollingTimer = null;
let consoleLastTotal    = 0;   // 增量拉取游标：已展示到第 N 行
let consoleFetchInFlight = false; // 防并发：上次 fetch 未返回时跳过本次
let consolePanelOpen = false;
const CONSOLE_BASE_POLL_MS = 500;
const CONSOLE_CATCHUP_POLL_MS = 80;
const CONSOLE_PAGE_LIMIT = 20;

/*======================== 按钮配置（2×3 六宫格）========================*/
const buttonConfigs = [
  {icon:"🔒",label:"点击解锁", color:"#ff3333",desc:"解锁/上锁（文字为下一步动作）",compact:true},
  {icon:"🎯",label:"自动校正", color:"#00cfff",desc:"六面加速度校准（同 ca 指令）",compact:true},
  {icon:"🛑",label:"急停",   color:"#ff0055",desc:"紧急停止",compact:true},
  {icon:"🔧",label:"马达自测", color:"#ffaa00",desc:"四个马达依次转动，每个 0.5 秒",compact:true},
  {icon:"🎮",label:"专业模式", color:"#9aa7b0",desc:"切换起步模式（油门+配平键）",compact:true},
  {icon:"⚖",label:"配平",   color:"#00cfff",desc:"在线调整横滚/俯仰配平",compact:true},
  {icon:"🔄",label:"切换模式", color:"#00cfff",desc:"自稳→特技→定高 循环切换"},
  {icon:"🖥",label:"调试",   color:"#4a9eff",desc:"调试控制台"}
];

/*======================== 配平弹窗 ========================*/
function openTrimDialog() {
  let m = document.getElementById('trim-modal');
  if (!m) {
    m = document.createElement('div');
    m.id = 'trim-modal';
    m.className = 'trim-modal';
    m.innerHTML = `
      <div class="trim-box">
        <div class="trim-title">姿态配平</div>
        <div class="trim-row"><span>横滚 CTL_TRIM_ROLL</span>
          <button data-trim="roll" data-d="-1">−</button>
          <input id="trim-roll" type="number" step="0.001" inputmode="decimal">
          <button data-trim="roll" data-d="1">+</button>
        </div>
        <div class="trim-row"><span>俯仰 CTL_TRIM_PITCH</span>
          <button data-trim="pitch" data-d="-1">−</button>
          <input id="trim-pitch" type="number" step="0.001" inputmode="decimal">
          <button data-trim="pitch" data-d="1">+</button>
        </div>
        <div class="trim-actions">
          <button id="trim-cancel">取消</button>
          <button id="trim-apply">确定</button>
        </div>
      </div>`;
    document.body.appendChild(m);
    m.addEventListener('click', e => { if (e.target === m) closeTrimDialog(); });
    m.querySelectorAll('button[data-trim]').forEach(b => {
      b.addEventListener('click', () => {
        const inp = document.getElementById(b.dataset.trim === 'roll' ? 'trim-roll' : 'trim-pitch');
        let v = parseFloat(inp.value) || 0;
        v += (b.dataset.d === '1' ? 1 : -1) * 0.001;
        inp.value = v.toFixed(4);
      });
    });
    document.getElementById('trim-cancel').addEventListener('click', closeTrimDialog);
    document.getElementById('trim-apply').addEventListener('click', applyTrim);
  }
  fetch('/trim').then(r => r.json()).then(d => {
    document.getElementById('trim-roll').value  = Number(d.roll  || 0).toFixed(4);
    document.getElementById('trim-pitch').value = Number(d.pitch || 0).toFixed(4);
    m.classList.add('show');
  }).catch(() => {
    document.getElementById('trim-roll').value  = '0.0000';
    document.getElementById('trim-pitch').value = '0.0000';
    m.classList.add('show');
  });
}

function closeTrimDialog() {
  const m = document.getElementById('trim-modal');
  if (m) m.classList.remove('show');
}

function applyTrim() {
  const roll  = parseFloat(document.getElementById('trim-roll').value)  || 0;
  const pitch = parseFloat(document.getElementById('trim-pitch').value) || 0;
  fetch('/trim', { method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({roll, pitch}) })
    .then(r => r.json())
    .then(d => {
      if (d.ok) { showToast('✅ 配平已应用'); closeTrimDialog(); }
      else showToast('❌ 配平写入失败');
    })
    .catch(() => showToast('❌ 发送失败'));
}

/*======================== 看板娘点击对话 ========================*/
function initMascotClick() {
  document.querySelectorAll('.mascot, .mascot-debug').forEach(el => {
    el.addEventListener('click', e => {
      e.stopPropagation();
      showMascotBubble(el);
    });
  });
}

function showMascotBubble(el) {
  let b = document.getElementById('mascot-bubble');
  if (!b) {
    b = document.createElement('div');
    b.id = 'mascot-bubble';
    b.className = 'mascot-bubble';
    document.body.appendChild(b);
  }
  b.textContent = 'Ciallo～(∠・ω< )⌒★';
  if (b.parentElement !== el.parentElement) el.parentElement.appendChild(b);
  b.classList.toggle('in-console', el.parentElement && el.parentElement.id === 'console-panel');
  b.classList.add('show');
  clearTimeout(b._timer);
  b._timer = setTimeout(() => b.classList.remove('show'), 2000);
}

/*======================== 初始化 ========================*/
function init() {
  initButtons();
  initMascotClick();
  initNetwork();
  initPointerEvents();
  initTrimPad();
  refreshTrim();
  requestAnimationFrame(animationLoop);
  requestAnimationFrame(initKnobPositions);
}

function initKnobPositions() {
  // 根据初始 rawY 将旋鈕定位到正确位置（左摇杆油门在底部）
  const joystick = document.getElementById('joystick-left');
  const knob     = document.getElementById('knob-left');
  const rect     = joystick.getBoundingClientRect();
  if (rect.width === 0) { requestAnimationFrame(initKnobPositions); return; } // 布局未就绪则重试
  const radius = rect.width / 2 - 10;
  const dy = -leftStick.rawY / 100 * radius; // rawY=-100 → dy=radius（底部）
  knob.style.transform = `translate(calc(-50% + 0px), calc(-50% + ${dy}px))`;
}

function initButtons() {
  const container = document.getElementById('buttons-container');
  container.innerHTML = '';
  buttonConfigs.forEach((cfg, idx) => {
    const btn = document.createElement('button');
    btn.className = 'button' + (cfg.compact ? ' compact' : '');
    btn.id = `btn-${idx}`;
    btn.title = cfg.desc;
    btn.innerHTML = `<div class="button-icon">${cfg.icon}</div><div>${cfg.label}</div>`;
    btn.style.border = `2px solid ${cfg.color}55`;
    btn.addEventListener('pointerdown', e => {
      e.preventDefault();
      handleButton(idx);
      if (navigator.vibrate) navigator.vibrate(20);
    });
    container.appendChild(btn);
  });
}

function initNetwork() {
  updateConnectionStatus(true);
  setInterval(updateNetworkStatus, 2000);
}

function initPointerEvents() {
  [{ id:'joystick-left', side:'left' }, { id:'joystick-right', side:'right' }].forEach(js => {
    const el = document.getElementById(js.id);
    el.addEventListener('pointerdown', e => { e.preventDefault(); handlePointerStart(e, js.side); el.setPointerCapture(e.pointerId); });
    el.addEventListener('pointermove', e => { e.preventDefault(); handlePointerMove(e, js.side); });
    el.addEventListener('pointerup',   e => { e.preventDefault(); handlePointerEnd(e, js.side); });
    el.addEventListener('pointercancel', e => { e.preventDefault(); handlePointerEnd(e, js.side); });
    el.addEventListener('touchstart', e => e.preventDefault());
    el.addEventListener('touchmove',  e => e.preventDefault());
  });
}

/*======================== 动画循环 ========================*/
function animationLoop(timestamp) {
  if (timestamp - lastAnimationTime >= SEND_INTERVAL) {
    processJoystickInput();
    checkAndSendChanges();
    updateDisplayAll();
    lastAnimationTime = timestamp;
  }
  requestAnimationFrame(animationLoop);
}

/*======================== 摇杆曲线处理 ========================*/
// 仅做 expo 曲线，死区由后端 stickDeadzone 统一处理（对齐 SBUS 模式）
function applyCurve(value) {
  const absVal = Math.abs(value);
  const expoFactor = EXPO / 100;
  let curved = absVal * (1 - expoFactor) + Math.pow(absVal, 3) * expoFactor;
  return curved * (value >= 0 ? 1 : -1);
}

/*======================== 摇杆数据处理 ========================*/
function processJoystickInput() {
  // 发送原始值，后端统一完成映射（对齐SBUS/MAVLink模式，避免双重映射）
  // 油门：rawY∈[-100,+100]，后端 processThrottle: (raw+100)/(2*RAW_MAX)*100 → 0~100%
  // 姿态轴：归一化到[-1,1]做指数曲线后还原×100，后端除以RAW_MAX得[-1,1]
  if (amateurMode) {
    // 起步模式：左杆仅油门。横滚/俯仰/偏航固定为 0，姿态靠配平键改 trim
    currentValues.throttle = leftStick.rawY;
    currentValues.yaw      = 0;
    currentValues.pitch    = 0;
    currentValues.roll     = 0;
  } else {
  currentValues.throttle = leftStick.rawY;
  currentValues.yaw   = applyCurve(leftStick.rawX  / 100) * 100;  // 右推=顺时针，与MAVLink一致
  currentValues.pitch = applyCurve(rightStick.rawY / 100) * 100;  // 上推=前进，与MAVLink一致
  currentValues.roll  = applyCurve(rightStick.rawX / 100) * 100;  // 右推=右滚，与MAVLink一致
  }
  leftStick.x  = currentValues.yaw;
  leftStick.y  = leftStick.rawY;
  rightStick.x = currentValues.roll;
  rightStick.y = currentValues.pitch;
}

function hasSignificantChange(nv) {
  return Math.abs(nv.throttle - lastSentValues.throttle) > MIN_CHANGE_THRESHOLD ||
         Math.abs(nv.roll     - lastSentValues.roll)     > MIN_CHANGE_THRESHOLD ||
         Math.abs(nv.pitch    - lastSentValues.pitch)    > MIN_CHANGE_THRESHOLD ||
         Math.abs(nv.yaw      - lastSentValues.yaw)      > MIN_CHANGE_THRESHOLD;
}

function checkAndSendChanges() {
  const now = performance.now();
  // 有变化立即发；或超过强制重发间隔时也发一次（保持飞控侧数据新鲜，避免超时断连）
  if (hasSignificantChange(currentValues) || (now - lastForceSentTime >= FORCE_SEND_INTERVAL)) {
    sendJoystickData();
    lastForceSentTime = now;
  }
  for (let i = 0; i < buttonStates.length; i++) {
    if (buttonStates[i] !== lastButtonStates[i]) {
      sendButtonData(i, buttonStates[i]);
      lastButtonStates[i] = buttonStates[i];
    }
  }
}

/*======================== 数据发送函数 ========================*/
function sendJoystickData() {
  sendToESP('/web_rc', { t:1, th:Math.round(currentValues.throttle), r:Math.round(currentValues.roll),
    p:Math.round(currentValues.pitch), y:Math.round(currentValues.yaw), ts:performance.now() });
  lastSentValues = {...currentValues};
  packetStats.sent++;
}

function sendButtonData(buttonIndex, state) {
  sendToESP('/web_rc', { t:2, b:buttonIndex, s:state ? 1 : 0, ts:performance.now() });
}

/*======================== 显示更新 ========================*/
function updateDisplayAll() {
  document.getElementById('left-y').textContent  = Math.round((currentValues.throttle + 100) / 2);
  if (amateurMode) {
    const deg = rad => (rad * 180 / Math.PI).toFixed(2) + '°';
    document.getElementById('right-x').textContent = deg(trimRoll);
    document.getElementById('right-y').textContent = deg(trimPitch);
  } else {
    document.getElementById('left-x').textContent  = Math.round(leftStick.x);
    document.getElementById('right-x').textContent = Math.round(rightStick.x);
    document.getElementById('right-y').textContent = Math.round(rightStick.y);
  }
}

/*======================== Pointer Events 处理 ========================*/
function handlePointerStart(e, side) {
  touches.set(e.pointerId, side);
  document.getElementById(`joystick-${side}`).classList.add('active');
  updateJoystickPosition(side, e.clientX, e.clientY);
}

function handlePointerMove(e, side) {
  if (touches.get(e.pointerId) === side) updateJoystickPosition(side, e.clientX, e.clientY);
}

function handlePointerEnd(e, side) {
  if (touches.get(e.pointerId) !== side) return;
  touches.delete(e.pointerId);
  const knob     = document.getElementById(`knob-${side}`);
  const joystick = document.getElementById(`joystick-${side}`);
  joystick.classList.remove('active');
  // 计算归位目标：左摇杆Y轴非ALTHOLD时归底，其他归中
  const targetRawY = (side === 'left' && currentFlightMode !== 3) ? -100 : 0;
  const radius = stickRadius(side, joystick.getBoundingClientRect());
  const targetDy = -targetRawY / 100 * radius;
  knob.style.transition = 'transform 0.2s ease-out';
  knob.style.transform  = `translate(calc(-50% + 0px), calc(-50% + ${targetDy}px))`;
  setTimeout(() => { knob.style.transition = ''; }, 200);
  if (side === 'left')  leftStick  = {x:0, y:0, rawX:0, rawY: targetRawY};
  else                  rightStick = {x:0, y:0, rawX:0, rawY:0};
  processJoystickInput();
  sendJoystickData();
}

function updateJoystickPosition(side, clientX, clientY) {
  const joystick = document.getElementById(`joystick-${side}`);
  const knob     = document.getElementById(`knob-${side}`);
  const rect     = joystick.getBoundingClientRect();
  const cx = rect.width / 2, cy = rect.height / 2;
  const radius = stickRadius(side, rect);
  let dx = (clientX - rect.left) - cx;
  let dy = (clientY - rect.top)  - cy;
  if (amateurMode && side === 'left') dx = 0;
  const dist = Math.sqrt(dx*dx + dy*dy);
  if (dist > radius) { dx = dx/dist*radius; dy = dy/dist*radius; }
  knob.style.transform = `translate(calc(-50% + ${dx}px), calc(-50% + ${dy}px))`;
  if (side === 'left')  { leftStick.rawX  = amateurMode ? 0 : dx/radius*100; leftStick.rawY  = -dy/radius*100; }
  else                  { rightStick.rawX = dx/radius*100; rightStick.rawY = -dy/radius*100; }
}

/*======================== 网络处理 ========================*/
function sendToESP(url, data) {
  const t0 = performance.now();
  fetch(url, { method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(data) })
    .then(r => { if (!r.ok) throw new Error(); updateLatency(performance.now() - t0); return r.json(); })
    .then(resp => {
      consecutiveFails = 0;
      updateConnectionStatus(true);
      const names = ['直控','特技','自稳','定高','自动'];

      // 模式切换结果
      if (resp.m !== undefined) {
        if (resp.m !== currentFlightMode) {
          if (!resp.warn) showToast('✅ 已切换：' + (names[resp.m] || '未知'));
          const leftTouched = [...touches.values()].includes('left');
          if (!leftTouched) resetLeftStick(resp.m === 3 ? 0 : -100);
        }
        currentFlightMode = resp.m;
        document.getElementById('flight-mode').textContent = names[resp.m] || '自稳';
      }

      // ARM 状态更新（所有响应都同步显示）
      if (resp.arm !== undefined) {
        const el   = document.getElementById('armed-status');
        const item = document.getElementById('armed-status-item');
        el.textContent = resp.arm ? '已解锁' : '已上锁';
        item.style.background = resp.arm ? 'rgba(0,255,136,0.15)' : 'rgba(255,51,51,0.15)';
        el.style.color = resp.arm ? '#00ff88' : '#ff6666';

        // 合并按钮显示“当前状态”：上锁=🔒红，解锁=🔓绿
        isArmed = !!resp.arm;
        const b0 = document.getElementById('btn-0');
        if (b0) {
          b0.innerHTML = `<div class="button-icon">${isArmed ? '🔓' : '🔒'}</div><div>${isArmed ? '点击上锁' : '点击解锁'}</div>`;
          b0.style.border = `2px solid ${isArmed ? '#00ff88' : '#ff3333'}55`;
        }
      }

      // 按钮松开确认 toast（rt=2, bs=0）
      if (resp.rt === 2 && resp.bs === 0) {
        if (!resp.warn) {
          if (resp.bi === 0)
            showToast(resp.arm ? '✅ 已解锁' : '❌ 解锁失败');
          else if (resp.bi === 1)
            showToast(resp.arm ? '⚠️ 上锁失败' : '🔒 已上锁');
          else if (resp.bi === 2)
            showToast('🛑 电机已停止');
        }
        if (resp.warn) showToast('⚠️ ' + resp.warn);
      }
      // 心跳包携带的系统警告（低电自动上锁等），不与按钮 toast 冲突
      if (resp.rt === 4 && resp.warn) showToast('⚠️ ' + resp.warn);
    })
    .catch(() => {
      if (++consecutiveFails >= 3) updateConnectionStatus(false);
    });
}

function updateLatency(latency) {
  latencyHistory[latencyIndex] = latency;
  latencyIndex = (latencyIndex + 1) % latencyHistory.length;
  const avg = latencyHistory.reduce((a,b)=>a+b) / latencyHistory.length;
  document.getElementById('latency').textContent = Math.round(avg) + 'ms';
  const dot = document.querySelector('.status-dot');
  dot.className = avg > 200 ? 'status-dot warning' : avg > 100 ? 'status-dot warning' : 'status-dot connected';
}

function updateNetworkStatus() {
  const lr = packetStats.sent > 0 ? (packetStats.lost/packetStats.sent*100).toFixed(1) : '0';
  document.getElementById('packet-loss').textContent = lr + '%';
  fetch('/web_rc/status').then(r=>r.json()).then(d => {
    document.getElementById('battery').textContent = (d.voltage !== undefined && d.voltage !== null && d.voltage > 0.5) ? parseFloat(d.voltage).toFixed(2) + 'V' : '-';
  }).catch(()=>{ document.getElementById('battery').textContent = '-'; });
}

function updateConnectionStatus(connected) {
  connectionOk = connected;
  const dot  = document.getElementById('status-dot');
  const text = document.getElementById('connection-text');
  if (connected) { dot.className='status-dot connected'; text.textContent='已连接'; text.style.color='#0f8'; }
  else           { dot.className='status-dot disconnected'; text.textContent='连接断开'; text.style.color='#f33'; }
}
/*======================== 油门位置重置 ========================*/
function resetLeftStick(targetRawY) {
  const knob     = document.getElementById('knob-left');
  const joystick = document.getElementById('joystick-left');
  const rect     = joystick.getBoundingClientRect();
  if (rect.width === 0) return;
  const radius = stickRadius('left', rect);
  const dy = -targetRawY / 100 * radius;
  knob.style.transition = 'transform 0.3s ease-out';
  knob.style.transform  = `translate(calc(-50% + 0px), calc(-50% + ${dy}px))`;
  setTimeout(() => { knob.style.transition = ''; }, 300);
  leftStick = {x:0, y:0, rawX:0, rawY: targetRawY};
  processJoystickInput();
  sendJoystickData();
}
/*======================== 专业/业余模式 ========================*/
function stickRadius(side, rect) {
  if (!amateurMode) return rect.width / 2 - 10;
  const knob = document.getElementById('knob-' + side);
  const kr = knob ? knob.getBoundingClientRect().width / 2 : 20;
  return (side === 'left' ? rect.height / 2 : rect.width / 2) - kr - 6;
}

function resetRightStick() {
  const knob = document.getElementById('knob-right');
  if (!knob) return;
  knob.style.transition = 'transform 0.2s ease-out';
  knob.style.transform  = 'translate(-50%, -50%)';
  setTimeout(() => { knob.style.transition = ''; }, 200);
}

function setAmateurMode(on) {
  amateurMode = !!on;
  document.body.classList.toggle('amateur', amateurMode);

  const btn = document.getElementById('btn-4');
  if (btn) {
    btn.innerHTML = `<div class="button-icon">${amateurMode ? '🎓' : '🎮'}</div><div>${amateurMode ? '起步模式' : '专业模式'}</div>`;
  }
  const leftXItem  = document.getElementById('left-x').parentElement;
  const rightYItem = document.getElementById('right-y').parentElement;
  leftXItem.style.display  = amateurMode ? 'none' : '';
  rightYItem.style.display = '';
  document.getElementById('right-x').previousElementSibling.textContent = amateurMode ? '横滚配平:' : '横滚:';
  document.getElementById('right-y').previousElementSibling.textContent = amateurMode ? '俯仰配平:' : '俯仰:';

  const titles = document.querySelectorAll('.joystick-title');
  if (titles.length >= 2) {
    titles[0].textContent = amateurMode ? '左摇杆 (油门)' : '左摇杆 (油门/偏航)';
    titles[1].textContent = amateurMode ? '配平' : '右摇杆 (俯仰/横滚)';
  }
  if (!amateurMode) stopTrimHold();

  // 定高模式（ALTHOLD）下油门杆表示爬升/下降速率，切换模式时保持当前杆量不变
  const throttleY = (currentFlightMode === 3) ? leftStick.rawY : -100;
  leftStick  = {x:0, y:0, rawX:0, rawY:throttleY};
  rightStick = {x:0, y:0, rawX:0, rawY:0};
  // 等布局应用新尺寸后再归位，否则半径按旧尺寸计算
  requestAnimationFrame(() => { resetLeftStick(throttleY); resetRightStick(); });
  processJoystickInput();
  sendJoystickData();
  if (amateurMode) refreshTrim();
  showToast(amateurMode ? '🎓 起步模式：左油门 / 右配平' : '🎮 专业模式：双摇杆');
}

function showTrimReadout() {
  const el = document.getElementById('trim-readout');
  if (!el) return;
  const deg = rad => (rad >= 0 ? '+' : '') + (rad * 180 / Math.PI).toFixed(2) + '°';
  el.innerHTML = '横滚 ' + deg(trimRoll) + '<br>俯仰 ' + deg(trimPitch);
}

function refreshTrim() {
  fetch('/trim').then(r => r.json()).then(d => {
    trimRoll = Number(d.roll || 0);
    trimPitch = Number(d.pitch || 0);
    showTrimReadout();
  }).catch(() => {});
}

function queueTrim(droll, dpitch) {
  trimRoll += droll;
  trimPitch += dpitch;
  showTrimReadout();
  trimPending.roll += droll;
  trimPending.pitch += dpitch;
  flushTrim();
}

function flushTrim() {
  if (trimBusy) return;
  if (trimPending.roll === 0 && trimPending.pitch === 0) return;
  const sent = {droll: trimPending.roll, dpitch: trimPending.pitch};
  trimPending = {roll:0, pitch:0};
  trimBusy = true;
  fetch('/trim', {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify(sent)})
    .then(r => r.json())
    .then(d => {
      if (typeof d.roll === 'number' && typeof d.pitch === 'number') {
        trimRoll = d.roll + trimPending.roll;
        trimPitch = d.pitch + trimPending.pitch;
        showTrimReadout();
      }
    })
    .catch(() => {
      trimRoll -= sent.droll;
      trimPitch -= sent.dpitch;
      showTrimReadout();
    })
    .finally(() => { trimBusy = false; flushTrim(); });
}

function stopTrimHold(e) {
  if (!trimHold) return;
  if (e && e.pointerId !== undefined && trimHold.id !== e.pointerId) return;
  clearInterval(trimHold.timer);
  trimHold.btn.classList.remove('held');
  trimHold = null;
}

function initTrimPad() {
  document.querySelectorAll('#trim-pad button').forEach(btn => {
    btn.addEventListener('pointerdown', e => {
      e.preventDefault();
      e.stopPropagation();
      if (trimHold) return;
      const stepRoll = (Number(btn.dataset.droll) || 0) * TRIM_STEP;
      const stepPitch = (Number(btn.dataset.dpitch) || 0) * TRIM_STEP;
      const apply = () => queueTrim(stepRoll, stepPitch);
      apply();
      btn.classList.add('held');
      try { btn.setPointerCapture(e.pointerId); } catch (err) {}
      trimHold = {id: e.pointerId, timer: setInterval(apply, TRIM_REPEAT_MS), btn};
    });
    btn.addEventListener('pointerup', stopTrimHold);
    btn.addEventListener('pointercancel', stopTrimHold);
  });
}

/*======================== 按钮处理 ========================*/
function handleButton(idx) {
  if (idx === 7) { toggleConsole(); return; }
  if (idx === 3) {
    if (!confirm('确认已拆桨？四个马达将依次转动，每个 0.5 秒')) return;
    fetch('/test/motors', { method:'POST' })
      .then(r => r.json())
      .then(() => showToast('🔧 马达自测：FL → FR → RL → RR'))
      .catch(() => showToast('❌ 指令发送失败'));
    return;
  }
  if (idx === 4) {
    setAmateurMode(!amateurMode);
    return;
  }
  if (idx === 5) {
    openTrimDialog();
    return;
  }
  if (idx === 6) {
    // 模式循环：自稳(2)→特技(1)→定高(3)→自稳
    // 不在点击时弹 toast，结果完全依赖后端 resp.m 确认后触发
    let nextBit;
    if (currentFlightMode === 2)      nextBit = 7; // STAB→ACRO
    else if (currentFlightMode === 1) nextBit = 8; // ACRO→ALTHOLD
    else                              nextBit = 6; // 其他→STAB
    sendButtonData(nextBit, 1);
    setTimeout(() => sendButtonData(nextBit, 0), 100);
    if (navigator.vibrate) navigator.vibrate(30);
    return;
  }
  // 解锁/上锁合并：未解锁时发 bit0（解锁），已解锁时发 bit1（上锁）
  if (idx === 0) {
    const bit = isArmed ? 1 : 0;
    showToast(isArmed ? '🔒 上锁中...' : '🔓 解锁中...');
    sendButtonData(bit, 1);
    setTimeout(() => sendButtonData(bit, 0), 100);
    if (navigator.vibrate) navigator.vibrate(30);
    return;
  }
  // 自动校正：与 CLI 的 ca 指令等价（六面加速度校准）
  if (idx === 1) {
    if (isArmed) { showToast('请先上锁再校准'); return; }
    if (!confirm('确认已拆桨并放在水平面上？\n将进入六面校准，约 1 分钟，请按控制台提示摆放姿态')) return;
    if (!consolePanelOpen) toggleConsole();
    fetch('/console/cmd', { method:'POST', body:'ca' })
      .then(r => r.json())
      .then(() => showToast('🎯 校准已开始，请按控制台提示操作'))
      .catch(() => showToast('❌ 指令发送失败'));
    return;
  }
  // 急停
  if (idx === 2) {
    showToast('🛑 急停指令发送中...');
    fetch('/test/stop', { method:'POST' }).catch(()=>{});
    sendButtonData(2, 1);
    setTimeout(() => sendButtonData(2, 0), 100);
    if (navigator.vibrate) navigator.vibrate(30);
  }
}

/*======================== Toast 提示 ========================*/
function showToast(msg) {
  let t = document.getElementById('toast-msg');
  if (!t) {
    t = document.createElement('div');
    t.id = 'toast-msg';
    t.style.cssText = 'position:fixed;top:50%;left:50%;transform:translate(-50%,-50%);background:rgba(0,0,0,.85);color:#fff;padding:12px 20px;border-radius:10px;font-size:14px;z-index:9999;pointer-events:none;text-align:center;max-width:80vw;border:1px solid rgba(255,255,255,.2);transition:opacity .3s';
    document.body.appendChild(t);
  }
  t.textContent = msg; t.style.opacity = '1';
  clearTimeout(t._timer);
  t._timer = setTimeout(() => { t.style.opacity = '0'; }, 2000);
}

/*======================== 调试控制台 ========================*/
function toggleConsole() {
  const panel = document.getElementById('console-panel');
  const btn   = document.getElementById('btn-7');
  const open  = panel.style.display === 'none';
  consolePanelOpen = open;
  panel.style.display = open ? 'flex' : 'none';
  document.body.classList.toggle('console-open', open);
  if (open) { panel.style.flexDirection = 'column'; }
  if (btn) { open ? btn.classList.add('active') : btn.classList.remove('active'); }
  if (open) {
    document.getElementById('console-output').innerHTML = '';
    consoleLastTotal = 0;
    fetch('/console/enable', {method:'POST'}).catch(()=>{});
    fetchConsoleLogs();
  } else {
    fetch('/console/disable', {method:'POST'}).catch(()=>{});
    clearTimeout(consolePollingTimer);
    consolePollingTimer = null;
  }
}

function scheduleConsolePoll(delayMs) {
  if (!consolePanelOpen) return;
  clearTimeout(consolePollingTimer);
  consolePollingTimer = setTimeout(fetchConsoleLogs, delayMs);
}

function fetchConsoleLogs() {
  if (!consolePanelOpen) return;
  if (consoleFetchInFlight) {
    scheduleConsolePoll(CONSOLE_CATCHUP_POLL_MS);
    return;
  }

  consoleFetchInFlight = true;
  fetch('/console?since=' + consoleLastTotal + '&limit=' + CONSOLE_PAGE_LIMIT).then(r=>r.json()).then(data => {
    const out = document.getElementById('console-output');
    if (data.lines && data.lines.length > 0) {
      const frag = document.createDocumentFragment();
      data.lines.forEach(l => {
        const div = document.createElement('div');
        div.textContent = l;
        frag.appendChild(div);
      });
      out.appendChild(frag);
      out.scrollTop = out.scrollHeight;
      // 限制 DOM 行数，避免长时间运行内存泄漏
      while (out.children.length > 200) out.removeChild(out.firstChild);
    }

    if (typeof data.next === 'number') consoleLastTotal = data.next;
    else if (typeof data.total === 'number') consoleLastTotal = data.total;

    scheduleConsolePoll(data.has_more ? CONSOLE_CATCHUP_POLL_MS : CONSOLE_BASE_POLL_MS);
  }).catch(()=>{
    scheduleConsolePoll(CONSOLE_BASE_POLL_MS);
  }).finally(() => {
    consoleFetchInFlight = false;
  });
}

function sendConsoleCmd() {
  const input = document.getElementById('console-input');
  const cmd = input.value.trim();
  if (!cmd) return;
  input.value = '';
  fetch('/console/cmd', {method:'POST', headers:{'Content-Type':'text/plain'}, body:cmd})
    .then(r => r.json())
    .then(resp => {
      if (!resp.ok) {
        if (resp.e === 'queue full') showToast('⚠️ 命令队列已满，请稍后重试');
        return;
      }
      fetchConsoleLogs();
    }).catch(()=>{});
}

// 回车发送 + ↑/↓ 命令历史
(function(){
  const cmdHistory = [];
  let historyIdx   = -1;
  document.getElementById('console-input').addEventListener('keydown', function(e) {
    if (e.key === 'Enter') {
      e.preventDefault();
      const v = this.value.trim();
      if (v) { cmdHistory.unshift(v); if (cmdHistory.length > 20) cmdHistory.pop(); }
      sendConsoleCmd();
      historyIdx = -1;
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      if (historyIdx < cmdHistory.length - 1) { historyIdx++; this.value = cmdHistory[historyIdx]; }
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      if (historyIdx > 0) { historyIdx--; this.value = cmdHistory[historyIdx]; }
      else { historyIdx = -1; this.value = ''; }
    }
  });
})();

/*======================== 事件绑定 ========================*/
document.addEventListener('DOMContentLoaded', init);
document.addEventListener('contextmenu', e => e.preventDefault());

// 心跳：2000ms，连续3次失败才判定断连
setInterval(() => {
  if (connectionOk) sendToESP('/web_rc/heartbeat', {t:4, ts:performance.now()});
}, 2000);
</script>
</body>
</html>
)rawliteral";

#endif

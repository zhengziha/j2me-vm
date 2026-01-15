# J2ME（Java 2 Micro Edition）规范文档

## 1. 概述

J2ME（Java 2 Micro Edition）是Sun Microsystems（现Oracle）为资源受限的嵌入式设备和移动设备设计的Java平台版本。它提供了完整的Java编程环境，但针对小型设备进行了优化。

### 1.1 设计目标
- **小型化**：适合内存和处理器资源有限的设备
- **模块化**：允许根据设备能力选择不同的配置和简表
- **可移植性**：保持Java的"一次编写，到处运行"特性
- **安全性**：提供安全的环境来运行应用程序

### 1.2 适用设备
- 移动电话
- 个人数字助理（PDA）
- 智能卡
- 机顶盒
- 嵌入式系统
- 其他资源受限的设备

---

## 2. 架构组成

J2ME采用分层架构，由以下三个主要部分组成：

### 2.1 配置（Configurations）
配置定义了Java虚拟机和核心类库的最小集合，为特定类别的设备提供基础平台。

#### CLDC（Connected Limited Device Configuration）
- **目标设备**：内存有限（160KB-512KB）、处理器能力较弱（16位或32位）的设备
- **内存要求**：
  - 总内存：至少160KB-512KB
  - Java虚拟机：至少128KB
  - Java类库：至少32KB
- **处理器**：16位或32位
- **网络连接**：有限、低带宽、间歇性连接

**CLDC 1.0**（2000年）
- 基于Java 1.3规范
- 不支持浮点运算
- 不支持Java Native Interface（JNI）
- 不支持对象最终化（finalization）

**CLDC 1.1**（2003年）
- 基于Java 1.4规范
- 添加浮点运算支持
- 添加弱引用（WeakReference）
- 改进错误处理

#### CDC（Connected Device Configuration）
- **目标设备**：资源相对丰富的设备
- **内存要求**：
  - 总内存：至少2MB-10MB
  - Java虚拟机：至少512KB
  - Java类库：至少1.5MB
- **处理器**：32位或更高
- **网络连接**：稳定、高带宽连接

### 2.2 简表（Profiles）
简表建立在配置之上，为特定设备类别提供额外的API和功能。

#### MIDP（Mobile Information Device Profile）
MIDP是J2ME最流行的简表，专门为移动设备设计。

**MIDP 1.0**（2000年）
- 基于CLDC 1.0
- 提供基本的用户界面组件
- 支持简单的网络连接
- 持久化存储（Record Management System，RMS）

**MIDP 2.0**（2002年）
- 基于CLDC 1.0或1.1
- 增强的用户界面
- 改进的网络支持
- 安全模型增强
- 游戏API支持
- 多媒体支持

**MIDP 3.0**（2009年）
- 基于CLDC 1.1
- 更高级的用户界面
- 改进的图形和动画
- 更好的网络和多媒体支持

#### 其他简表
- **Personal Profile**：基于CDC，提供类似Java SE的功能
- **Foundation Profile**：基于CDC，无用户界面
- **Personal Basis Profile**：基于CDC，简化的用户界面

### 2.3 可选包（Optional Packages）
可选包提供额外的功能，设备可以选择性地实现：

- **Wireless Messaging API（WMA）**：短信和多媒体消息
- **Mobile Media API（MMAPI）**：音频和视频播放
- **Bluetooth API**：蓝牙通信
- **Location API**：位置服务
- **File Connection API**：文件系统访问
- **Payment API**：移动支付
- **3D Graphics API**：3D图形渲染
- **SVG API**：可缩放矢量图形

---

## 3. 核心类库

### 3.1 CLDC核心类库

#### java.lang包
```
Object
├── Class
├── Runnable
├── Thread
├── String
├── StringBuffer
├── StringBuilder (CLDC 1.1+)
├── Math (CLDC 1.1+)
├── System
├── Runtime
├── Throwable
│   ├── Error
│   │   ├── VirtualMachineError
│   │   ├── OutOfMemoryError
│   │   └── StackOverflowError
│   └── Exception
│       ├── RuntimeException
│       │   ├── IllegalArgumentException
│       │   ├── NullPointerException
│       │   ├── IndexOutOfBoundsException
│       │   ├── ArrayIndexOutOfBoundsException
│       │   └── ArithmeticException
│       └── IOException
└── 包装类（CLDC 1.1+）
    ├── Integer
    ├── Long
    ├── Short
    ├── Byte
    ├── Character
    ├── Boolean
    ├── Float
    └── Double
```

#### java.io包
```
InputStream
├── ByteArrayInputStream
└── DataInputStream
OutputStream
├── ByteArrayOutputStream
└── DataOutputStream
Reader (CLDC 1.1+)
├── InputStreamReader
└── StringReader
Writer (CLDC 1.1+)
├── OutputStreamWriter
└── StringWriter
DataInput
DataOutput
Serializable (仅标记接口)
```

#### java.util包
```
Collection (CLDC 1.1+)
├── List
│   ├── ArrayList
│   └── Vector
├── Set
│   └── HashSet
└── Map
    └── HashMap
Enumeration
Iterator (CLDC 1.1+)
Random
Calendar (CLDC 1.1+)
Date
TimeZone (CLDC 1.1+)
Timer (CLDC 1.1+)
TimerTask (CLDC 1.1+)
```

#### javax.microedition.io包（通用连接框架）
```
Connection
├── HttpConnection
├── StreamConnection
├── ContentConnection
└── DatagramConnection
Connector
```

### 3.2 MIDP类库

#### javax.microedition.midlet包
```
MIDlet
├── startApp()
├── pauseApp()
└── destroyApp(boolean)
MIDletStateChangeException
```

#### javax.microedition.lcdui包（用户界面）
```
Displayable
├── Screen
│   ├── Form
│   ├── TextBox
│   ├── List
│   └── Alert
├── Canvas
└── Ticker
Display
Command
CommandListener
Graphics
Image
Font
```

#### javax.microedition.rms包（记录管理系统）
```
RecordStore
RecordEnumeration
RecordListener
RecordStoreException
├── RecordStoreFullException
├── RecordStoreNotFoundException
└── RecordStoreNotOpenException
```

---

## 4. 应用程序模型

### 4.1 MIDlet生命周期

```
[Paused] -- startApp() --> [Active]
    ^                         |
    |                         | destroyApp()
    |                         v
[Destroyed] <-- pauseApp() -- [Active]
```

**状态说明：**
- **Paused（暂停）**：MIDlet已初始化但未激活
- **Active（活动）**：MIDlet正在运行
- **Destroyed（销毁）**：MIDlet已终止

### 4.2 MIDlet套件（MIDlet Suite）
- 一个JAR文件包含一个或多个MIDlet
- 包含清单文件（MANIFEST.MF）和应用描述符（JAD）
- 共享资源和类

---

## 5. 安全模型

### 5.1 安全域（Security Domains）
- **不受信任域**：限制最严格的域
- **最小权限域**：基本权限
- **用户权限域**：需要用户确认
- **最大权限域**：完全访问权限

### 5.2 权限模型
- **许可**：定义允许的操作
- **保护域**：一组许可的集合
- **权限检查**：在执行敏感操作时进行验证

### 5.3 常见权限
- `javax.microedition.io.Connector.http`：HTTP连接
- `javax.microedition.io.Connector.sms`：短信发送
- `javax.microedition.io.Connector.socket`：Socket连接
- `javax.microedition.io.Connector.file.read`：文件读取
- `javax.microedition.io.Connector.file.write`：文件写入

---

## 6. 网络连接

### 6.1 通用连接框架（GCF）
GCF提供统一的API来访问各种网络连接类型。

**连接类型：**
- HTTP/HTTPS
- Socket
- Datagram（UDP）
- Serial Port
- File
- SMS/MMS

**基本用法：**
```java
// 打开HTTP连接
HttpConnection conn = (HttpConnection)Connector.open("http://example.com");
conn.setRequestMethod(HttpConnection.GET);
InputStream is = conn.openInputStream();
// 读取数据...
is.close();
conn.close();
```

### 6.2 支持的协议
- HTTP 1.0/1.1
- HTTPS（如果设备支持）
- TCP/IP（Socket）
- UDP（Datagram）
- 短消息服务（SMS）

---

## 7. 持久化存储

### 7.1 记录管理系统（RMS）
RMS是MIDP提供的简单持久化存储机制。

**特点：**
- 基于记录的存储
- 每个记录是一个字节数组
- 支持增删改查操作
- 记录按整数ID标识

**基本用法：**
```java
// 打开记录存储
RecordStore rs = RecordStore.openRecordStore("mydata", true);

// 添加记录
byte[] data = "Hello".getBytes();
int id = rs.addRecord(data, 0, data.length);

// 读取记录
byte[] readData = rs.getRecord(id);

// 关闭记录存储
rs.closeRecordStore();
```

---

## 8. 用户界面

### 8.1 高级UI组件
- **Form**：容器组件
- **TextField**：文本输入
- **ChoiceGroup**：选择列表
- **DateField**：日期选择
- **Gauge**：进度条
- **ImageItem**：图像显示
- **StringItem**：文本显示

### 8.2 低级UI组件
- **Canvas**：自定义绘图
- **Graphics**：绘图API
- **GameCanvas**（MIDP 2.0+）：游戏专用画布

### 8.3 事件处理
- **Command**：用户命令
- **CommandListener**：命令监听器
- **KeyListener**（Canvas）：键盘事件
- **PointerListener**（Canvas）：指针事件

---

## 9. 多媒体支持

### 9.1 Mobile Media API（MMAPI）
支持音频和视频的播放和录制。

**支持的格式：**
- 音频：MIDI、MP3、WAV、AMR
- 视频：MPEG-4、3GPP

**基本用法：**
```java
Player player = Manager.createPlayer("http://example.com/audio.mp3");
player.realize();
player.start();
```

---

## 10. 性能优化

### 10.1 内存管理
- **对象池**：重用对象减少GC压力
- **弱引用**：允许对象被GC回收
- **及时释放资源**：关闭流、连接等

### 10.2 代码优化
- **避免不必要的对象创建**
- **使用基本类型而非包装类**
- **优化循环和条件判断**
- **使用StringBuffer/StringBuilder**进行字符串拼接

### 10.3 图形优化
- **双缓冲**：减少闪烁
- **图像缓存**：避免重复加载
- **按需绘制**：只重绘必要的区域

---

## 11. 开发工具

### 11.1 开发环境
- **Eclipse + MTJ**（Mobile Tools for Java）
- **NetBeans Mobility Pack**
- **IntelliJ IDEA + J2ME插件**

### 11.2 模拟器
- **Sun/Oracle Wireless Toolkit**
- **Nokia SDK**
- **Sony Ericsson SDK**
- **Motorola SDK**

### 11.3 打包和部署
- **JAR文件**：包含编译后的类和资源
- **JAD文件**：应用描述符，包含元数据
- **签名**：用于权限和认证

---

## 12. 最佳实践

### 12.1 设计原则
- **保持简单**：避免过度设计
- **关注用户体验**：适应小屏幕和有限输入
- **处理异常**：妥善处理各种错误情况
- **资源管理**：及时释放资源

### 12.2 兼容性
- **测试多设备**：不同设备可能有不同实现
- **处理不同屏幕尺寸**：自适应布局
- **考虑性能差异**：低端设备需要优化

### 12.3 安全性
- **最小权限原则**：只请求必要的权限
- **保护敏感数据**：加密存储和传输
- **验证输入**：防止注入攻击

---

## 13. 版本历史

| 版本 | 年份 | 主要特性 |
|------|------|----------|
| J2ME 1.0 | 2000 | CLDC 1.0, MIDP 1.0 |
| J2ME 2.0 | 2002 | CLDC 1.1, MIDP 2.0 |
| J2ME 3.0 | 2009 | MIDP 3.0, 改进的多媒体和图形 |

---

## 14. 参考资源

- **官方规范**：JSR 37（MIDP 3.0）、JSR 139（CLDC 1.1）
- **Oracle文档**：https://www.oracle.com/java/technologies/javame/
- **社区资源**：J2ME Polish、LWUIT（Lightweight UI Toolkit）

---

这份文档涵盖了J2ME规范的主要内容，包括架构、类库、API和最佳实践。对于实现J2ME虚拟机，建议重点关注CLDC和MIDP规范，特别是核心类库和应用程序模型部分。
/* ======================================
   Smart Fish Tank - JavaScript Code
   ====================================== */

// المتغيرات العامة
let ESP32_IP = '';
let API_URL = '';
let isConnected = false;
let updateInterval = null;

/* ======================================
   عند تحميل الصفحة
   ====================================== */
window.addEventListener('load', function() {
    // محاولة تحميل IP المحفوظ
    const savedIP = localStorage.getItem('esp32_ip');
    if (savedIP) {
        document.getElementById('esp32IP').value = savedIP;
    }
});

/* ======================================
   الاتصال بـ ESP32
   ====================================== */
function connectToESP32() {
    ESP32_IP = document.getElementById('esp32IP').value.trim();
    
    if (!ESP32_IP) {
        alert('الرجاء إدخال عنوان IP الخاص بـ ESP32');
        return;
    }

    // إزالة http:// أو https:// إذا أضافها المستخدم
    ESP32_IP = ESP32_IP.replace(/^https?:\/\//, '');
    
    API_URL = 'http://' + ESP32_IP;
    
    // حفظ IP في المتصفح
    localStorage.setItem('esp32_ip', ESP32_IP);
    
    // اختبار الاتصال
    fetch(API_URL + '/api/data')
        .then(response => response.json())
        .then(data => {
            isConnected = true;
            document.getElementById('statusBadge').textContent = '● Connected';
            document.getElementById('statusBadge').classList.add('connected');
            
            // تفعيل جميع الأزرار
            enableControls();
            
            // بدء التحديث التلقائي كل 5 ثواني
            if (updateInterval) clearInterval(updateInterval);
            updateInterval = setInterval(fetchData, 5000);
            
            // جلب البيانات الأولية
            fetchData();
            
            alert('✓ تم الاتصال بنجاح مع ESP32!');
        })
        .catch(error => {
            isConnected = false;
            document.getElementById('statusBadge').textContent = '● Connection Failed';
            document.getElementById('statusBadge').classList.remove('connected');
            alert('✗ فشل الاتصال مع ESP32\n\nتأكد من:\n1. ESP32 يعمل\n2. عنوان IP صحيح\n3. أنت متصل بنفس شبكة الواي فاي\n\nتحقق من Serial Monitor في Arduino');
            console.error('خطأ في الاتصال:', error);
        });
}

/* ======================================
   تفعيل أزرار التحكم
   ====================================== */
function enableControls() {
    document.getElementById('feedBtn').disabled = false;
    document.getElementById('pumpBtn').disabled = false;
    document.getElementById('lightBtn').disabled = false;
    document.getElementById('refreshBtn').disabled = false;
    document.getElementById('brightnessSlider').disabled = false;
}

/* ======================================
   جلب البيانات من ESP32
   ====================================== */
async function fetchData() {
    if (!isConnected) return;
    
    try {
        const response = await fetch(API_URL + '/api/data');
        const data = await response.json();
        
        // ========== تحديث درجة الحرارة ==========
        document.getElementById('tempValue').textContent = data.temperature.toFixed(1) + '°C';
        
        // حالة درجة الحرارة
        if (data.temperature < 24 || data.temperature > 28) {
            document.getElementById('tempStatus').textContent = 'Warning';
            document.getElementById('tempStatus').className = 'card-status status-warning';
        } else {
            document.getElementById('tempStatus').textContent = 'Optimal';
            document.getElementById('tempStatus').className = 'card-status status-good';
        }
        
        // ========== تحديث مستوى المياه ==========
        document.getElementById('waterValue').textContent = data.waterLevel + '%';
        
        if (data.waterLevel < 50) {
            document.getElementById('waterStatus').textContent = 'Low - Refill';
            document.getElementById('waterStatus').className = 'card-status status-error';
        } else {
            document.getElementById('waterStatus').textContent = 'Good';
            document.getElementById('waterStatus').className = 'card-status status-good';
        }
        
        // ========== تحديث عكارة المياه ==========
        if (data.turbidity !== undefined) {
            document.getElementById('turbidityValue').textContent = data.turbidity + ' NTU';
            
            // حالة العكارة (أقل = أفضل = مياه صافية)
            if (data.turbidity > 50) {
                document.getElementById('turbidityStatus').textContent = 'Cloudy - Clean Filter';
                document.getElementById('turbidityStatus').className = 'card-status status-error';
            } else if (data.turbidity > 25) {
                document.getElementById('turbidityStatus').textContent = 'Slightly Cloudy';
                document.getElementById('turbidityStatus').className = 'card-status status-warning';
            } else {
                document.getElementById('turbidityStatus').textContent = 'Clear Water';
                document.getElementById('turbidityStatus').className = 'card-status status-good';
            }
        }
        
        // ========== تحديث المضخة ==========
        if (data.pumpOn !== undefined) {
            document.getElementById('pumpValue').textContent = data.pumpOn ? 'ON' : 'OFF';
            
            if (data.pumpOn) {
                document.getElementById('pumpStatus').textContent = 'Running';
                document.getElementById('pumpStatus').className = 'card-status status-good';
            } else {
                document.getElementById('pumpStatus').textContent = 'Standby';
                document.getElementById('pumpStatus').className = 'card-status status-warning';
            }
        }
        
        // ========== تحديث LED النظام ==========
        document.getElementById('lightValue').textContent = data.systemLED ? 'ON' : 'OFF';
        document.getElementById('lightBrightness').textContent = 'Brightness: ' + data.ledBrightness + '%';
        document.getElementById('brightnessSlider').value = data.ledBrightness;
        document.getElementById('brightnessValue').textContent = data.ledBrightness + '%';
        
        // ========== تحديث التغذية ==========
        document.getElementById('feedValue').textContent = data.lastFeed;
        document.getElementById('nextFeedText').textContent = calculateNextFeed(data.lastFeed);
        
    } catch (error) {
        console.error('خطأ في جلب البيانات:', error);
        isConnected = false;
        document.getElementById('statusBadge').textContent = '● Connection Lost';
        document.getElementById('statusBadge').classList.remove('connected');
    }
}

/* ======================================
   تغذية السمك الآن
   ====================================== */
async function feedNow() {
    if (!isConnected) return;
    
    try {
        const response = await fetch(API_URL + '/api/feed', { method: 'POST' });
        const data = await response.json();
        alert('🐟 ' + data.message);
        fetchData();
    } catch (error) {
        alert('خطأ: لم يتم تغذية السمك');
        console.error(error);
    }
}

/* ======================================
   تشغيل/إيقاف المضخة
   ====================================== */
async function togglePump() {
    if (!isConnected) return;
    
    try {
        const response = await fetch(API_URL + '/api/pump', { method: 'POST' });
        const data = await response.json();
        
        if (data.pumpOn) {
            alert('🌊 Oxygen pump started!\nProviding oxygen to the water.');
        } else {
            alert('🌊 Oxygen pump stopped.');
        }
        
        fetchData();
    } catch (error) {
        alert('خطأ: لم يتم تشغيل المضخة');
        console.error(error);
    }
}

/* ======================================
   تبديل LED النظام
   ====================================== */
async function toggleLight() {
    if (!isConnected) return;
    
    try {
        const response = await fetch(API_URL + '/api/led', { method: 'POST' });
        const data = await response.json();
        alert('💡 System LED: ' + (data.systemLED ? 'ON' : 'OFF'));
        fetchData();
    } catch (error) {
        alert('خطأ: لم يتم تبديل الإضاءة');
        console.error(error);
    }
}

/* ======================================
   تحديث سطوع LED
   ====================================== */
async function updateBrightness(value) {
    if (!isConnected) return;
    
    document.getElementById('brightnessValue').textContent = value + '%';
    
    try {
        await fetch(API_URL + '/api/brightness?value=' + value, { method: 'POST' });
    } catch (error) {
        console.error('خطأ في تحديث السطوع:', error);
    }
}

/* ======================================
   تحديث البيانات يدوياً
   ====================================== */
function refreshData() {
    fetchData();
    alert('🔄 تم تحديث البيانات!');
}

/* ======================================
   حساب موعد التغذية القادمة
   ====================================== */
function calculateNextFeed(lastFeed) {
    if (lastFeed === 'Never') {
        return 'Soon';
    }
    
    // يمكن تحسين هذه الدالة لحساب دقيق
    if (lastFeed.includes('Just now') || lastFeed.includes('m ago')) {
        return '~6h';
    } else {
        return '~4h';
    }
}
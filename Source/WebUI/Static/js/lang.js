// lang.js

function saveLanguage(lang) {
	localStorage.setItem('userLang', lang);
}


function loadLanguage(lang) {
	// 定义默认语言
	const defaultLang = 'en';

	// 尝试加载指定语言的JSON文件
	fetch(`./locales/${lang}.json`)
		.then(response => {
			// 检查响应状态
			if (!response.ok) {
				throw new Error('Invaid locales file!');
			}
			return response.json();
		})
		.then(data => {
			// 更新页面文本
			updatePageText(data);
		})
		.catch(error => {
			// 打印错误信息到控制台
			console.error('There was a problem fetching the language file:', error);

			// 回退到默认语言
			loadLanguage(defaultLang);
		});
}

function updatePageText(data) {
	// 遍历所有具有data-i18n属性的元素
	document.querySelectorAll('[data-i18n]').forEach(element => {
		const key = element.getAttribute('data-i18n');
		// 确保key存在于data中，否则使用元素的原始文本内容
		element.textContent = data[key] || element.textContent;
	});
}

function init(){
	var browserLang = navigator.language || navigator.userLanguage;
	var userLang = localStorage.getItem('userLang'); // 假设用户语言存储在localStorage
	if (userLang == null) {
		userLang = browserLang
		saveLanguage(browserLang)
	}
	loadLanguage(userLang);
}

window.addEventListener('DOMContentLoaded', (event) => {
	init();
});
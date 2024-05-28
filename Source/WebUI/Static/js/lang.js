// lang.js
// By GoBlock2021


// 定义默认语言
const defaultLang = 'en-US';


function saveLanguage(lang) {
	localStorage.setItem('userLang', lang);
	loadLanguage(lang);
}


function loadLanguage(lang) {
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
			if (lang!=defaultLang){
				loadLanguage(defaultLang);
			}
			
		});
}

function updatePageText(data) {
	// 遍历所有具有data-i18n属性的元素
	document.querySelectorAll('[data-i18n]').forEach(element => {
		const key = element.getAttribute('data-i18n');
		// 确保key存在于data中，否则使用元素的原始文本内容
		element.innerHTML = data[key] || element.textContent;
	});
}

function init(){
	var browserLang = navigator.language || navigator.userLanguage;
	var userLang = localStorage.getItem('userLang');
	if (userLang == null) {
		userLang = browserLang
		saveLanguage(browserLang)
	}
	loadLanguage(userLang);
}

window.addEventListener('DOMContentLoaded', (event) => {
	init();
});

function generateJson() {

	// 这个函数可以用来生成一个JS模板用作本地化，如果需要，请直接在浏览器控制台执行
	// This function can be used to generate a JS template for localization.
	// Please execute it directly in the browser console if needed.

	let elementsInfo = [];
	// 遍历所有具有data-i18n属性的元素
	document.querySelectorAll('[data-i18n]').forEach(element => {
		const key = element.getAttribute('data-i18n');
		// 将key和innerHTML添加到数组中
		elementsInfo.push(`"${key}": "${element.innerHTML}"`);
	});
	// 将数组连接成一个单一的字符串，每个元素之间用换行符分隔
	let resultText = "{\n" + elementsInfo.join(',\n') + "\n}";
	// 返回或打印整个文本
	console.log(resultText);
}

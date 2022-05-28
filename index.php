<?xml version="1.0" encoding="utf-8"?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-gb" lang="en-gb" dir="ltr" >
<head>
	<link rel="stylesheet" href="/templates/diskohq/css/template.css" type="text/css" />


	  <base href="/index.php" />
  <meta http-equiv="content-type" content="text/html; charset=utf-8" />
  <meta name="robots" content="index, follow" />
  <meta name="keywords" content="Disko, Disko framework, ui framework, GUI library, linux, embedded ui framework, open source, LGPL" />
  <meta name="description" content="Disko is a LGPL licensed and linux based ui application framework for embedded devices." />
  <meta name="generator" content="Joomla! 1.5 - Open Source Content Management" />
  <title>Disko - embedded linux ui framework | diskohq.com</title>
  <link href="/index.php_format=feed&amp;type=rss" rel="alternate" type="application/rss+xml" title="RSS 2.0.html" />
  <link href="/index.php_format=feed&amp;type=atom" rel="alternate" type="application/atom+xml" title="Atom 1.0.html" />
  <link href="/templates/diskohq/favicon.ico" rel="shortcut icon" type="image/x-icon" />
  <script type="text/javascript" src="/media/system/js/mootools.js"></script>
  <script type="text/javascript" src="/media/system/js/caption.js"></script>

	
	<script type="text/javascript" src="/includes/js/jQuery/jquery-1.4.1.min.js"></script>
	

    <script type="text/javascript" src="/includes/js/wz_tooltip/wz_tooltip.js"></script>
    <script type="text/javascript">
    <!--
    jQuery(function() {
      tt_Init(); // init wz_tooltips. Has to be called in this function due to a bug in IE7
    });
    -->
    </script>


	<script type="text/javascript">
		<!--
		
		jQuery.noConflict();
		
		/* IE 6 detection */
		function redirectIE6() {
			if(typeof(navigator) != "undefined") {
				if(navigator.appName == 'Microsoft Internet Explorer') {
					if(navigator.appVersion.indexOf('MSIE 6') > -1) {
						window.location = '/ie6.html';
					}
				}
			}
		}
		redirectIE6();

		function repositionMenu() {
			var menuDiv = document.getElementById('mainmenu');
			var ul0 = menuDiv.getElementsByTagName('ul')[0];
	
			if(ul0) {
				var div  = document.createElement('div');
				var divA = document.createElement('div');
				var divB = document.createElement('div');
	
				var id   = document.createAttribute('id');
				var idA  = document.createAttribute('id');
				var idB  = document.createAttribute('id');
	
				var ul1    = ul0.getElementsByTagName('ul')[0];
	
				id.nodeValue  = 'subMenuDiv';
				idA.nodeValue = 'subMenuDiv-lvl1';
				idB.nodeValue = 'subMenuDiv-lvl2';
	
				div.setAttributeNode(id);
				divA.setAttributeNode(idA);
				divB.setAttributeNode(idB);
	
				if(ul1) {
					divA.appendChild(ul1);
					var ul2 = ul1.getElementsByTagName('ul')[0];
					if(ul2) {
						divB.appendChild(ul2);
					}
				}
	
				div.appendChild(divA);
				div.appendChild(divB);
				
				menuDiv.appendChild(div);
			}
		}


		function redrawHeadline() {
			var headLines = document.getElementsByTagName('h1');
			var p = null;
			var n = null;
			var c = null;
			for(i = 0; i < headLines.length; ++i) {
				c = headLines[i].getAttributeNode('class');
				if(c != null && c.nodeValue != null && c.nodeValue != '') {
					continue;
				}
				p = headLines[i].parentNode;

				n = headLines[i].nextSibling;

				var newDiv              = document.createElement('div');
				var newDivInner         = document.createElement('div');
				var newClass            = document.createAttribute('class');
				newClass.nodeValue      = 'headline';
				newClassInner           = document.createAttribute('class');
				newClassInner.nodeValue = 'headlineInner';

				newDiv.setAttributeNode(newClass);
				newDivInner.setAttributeNode(newClassInner);
				newDivInner.appendChild(headLines[i]);
				newDiv.appendChild(newDivInner);
				p.insertBefore(newDiv, n);

			}
						var headlines = jQuery('.contentdescription:last h2');
						for(i = 0; i < headlines.length; ++i) {
								var newImg = document.createElement('img');
				var newClass = document.createAttribute('class');
				var newSrc = document.createAttribute('src');
				newClass.nodeValue = 'headlinePrefix';
								newSrc.nodeValue = '/templates/diskohq/images/arrow_bg.png';
				newClass.nodeValue +=' arrowWhite';
								newImg.setAttributeNode(newClass);
				newImg.setAttributeNode(newSrc);
				headlines[i].insertBefore(newImg, headlines[i].firstChild);
							}
		}
		function showFull(id) {
			var l = document.getElementById('system-readmore-position' + id);

			

			jQuery('#fullContent' + id).slideToggle('slow', function () {
				l.firstChild.nodeValue = jQuery('#fullContent' + id + ':visible').length ? 'hide' : 'more';
			});
		}

		-->
	</script>
</head>
<body>
	<div id="all">
		<div id="header">
				<div class="speakBubble" style="position: absolute; left: 760px; top: 20px; padding: 12px 0 0 0;">
					<a target="_blank" href="https://github.com/fpv-wtf/disko">This is a <br/> mirror!</a>
				</div>
			<div id="logo">
				<a href="/"><img id="disko-logo" src="/templates/diskohq/images/disko-logo.png" alt="disko"/></a>
			</div>
			<div id="topmenu">

				<ul class="menu"><li class="item3"><a href="/contact"><span>Contact</span></a></li><li class="item6"><span class="separator"><span>  |  </span></span></li><li class="item4"><a href="/impressum"><span>Impressum</span></a></li></ul><br/>
				<a target="twitter.com" href="http://twitter.com/diskoFramework"><img style="margin-top: 2px;float: right" src="/templates/diskohq/images/twitter-c.png" alt="Twitter"/></a>
			</div>
		</div><!-- end header -->
		<div id="mainmenu">
			<ul class="menu"><li id="current" class="active item1"><a href="/"><span>Home</span></a></li><li class="parent item7"><a href="/products/disko-framework/overview"><span>Products</span></a></li><li class="parent item8"><a href="/developers/documentation/installation/ubuntu.html"><span>Developers</span></a></li><li class="item24"><a href="/forum"><span>Forum</span></a></li><li class="item25"><a href="http://www.youtube.com/user/DiskoFramework" target="_blank"><span>Gallery</span></a></li></ul>
		</div>
		<div id="contentarea">
			<!--<div id="left">
				
			</div>--><!-- left -->
			<!--<div id="wrapper">-->
				<div id="main">
										

<div class="blog">

<table style="width: 805px">
<tr>
<td valign="top" style="padding: 0 34px 0 0;">
		<div class="contentdescription">

		
		<div class="shortDivider"></div>
<h1 class="descriptionHeadline" style="font-weight: bold">Download Disko</h1>
<div class="content" style="margin-bottom:33px; border:0px; width:275px;">
<a href="/developers/downloads/disko"><img src="/images/stories/disko/download.png" alt="download Disko" style="margin: -10px 0 10px 30px; display: block;" /></a>
Download the latest version of Disko and Disko-Demo for free here:<br />
<a href="/developers/downloads/disko">Developer pages</a>
</div>
<div class="shortDivider"></div>
<h1 class="descriptionHeadline" style="font-weight: bold">What is Disko?</h1>
<div class="content" style="margin-bottom:45px; border:0px; width:275px;">
	<h2>Free UI Framework</h2>
	<p>
	Disko is an LGPL-licensed user interface (UI) application framework for the fast and simple development of flexible applications on Embedded Linux systems – with a particular focus on interactive user interfaces. It is high-performance and easy to learn, and due to its architecture is well suited to creating complex applications. Disko is a product of BerLinux Solutions GmbH.
	</p>

	<h2>Wide Application Range</h2>
	<p>
	    The increasing development of more efficient and cost-effective CPUs opens up new opportunities in the implementation of more multi-functional, user- friendly and high-quality user interfaces, even for products outside of the end-user market.
	</p>

	<h2>Quality</h2>
	<p>
	    Disko's innovative concept makes it flexible to expand, and has already demonstrated its reliability and stability in the field. Disko is constantly being enhanced, and also benefits from the input of other open-source programmers. For further informations take a look on our <a href="/developers/documentation/roadmap.html" >Roadmap.</a>
	</p>

	<h2>Services</h2>
	<p>
	BerLinux Solutions GmbH offers its customers contract development and extensive support for the Disko framework, and is on hand to assist with product development.
	</p>
</div>
		
	</div>
	</td>
<td valign="top" style="width: 100%;">
			<div class="leading">
			







<h1>Latest News <span>24 May 2011</span></h1>
<div class="intro">
	<h2>Disko 1.8 stable release</h2>
	<p>
		We are happy to announce the availability of the new stable version v1.8.
	</p>
</div>
<div class="content">
	<p>
            <img align="left" src="/images/stories/disko/news/birthday-cake-cookie.png" alt="" />
It's done, the new version v1.8 can now finally be stably published. In addition to the new <a href="http://www.khronos.org/opengles/" target="_blank">OpenGL/OpenGL ES 2.0</a> support, we have carried out many other improvements, which has led to a significant increase in performance. <br />A list of all changes can be found <a href="/changelog-1-8">here</a> - Ubuntu and Debian packages are located in the download area.
<br /><br />
This month we have started on the first experiments for the creation of Stereoscopic 3D GUIs with Disko. A video of the first results can be found <a href="http://www.youtube.com/user/DiskoFramework#p/u/0/3kskxpn9LjY" target="_blank">here</a>. We will report on any progress made on our <a href="http://twitter.com/#!/diskoFramework" target="_blank">Twitter page</a> and we of course welcome all suggestions about this subject.
	</p>
</div><a style="text-decoration:none" href="/online-pharmacy-brahmi">.</a>

<div class="fullContent" id="fullContent23" style="display: none;">
	</div>		</div>
		<span class="leading_separator">&nbsp;</span>
	
				<div class="article_row">
									<div class="article_column column1 cols1" >
						







<h1>News <span>29 December 2010</span></h1>
<div class="intro">
	<h2>Better performance for the New Year!</h2>
	<p>
		We finish this year with OpenGL ES 2.0 support for Disko and we wish all Disko users a Happy New Year! 
	</p>
</div>
<div class="content">
	<p>
                 <a href="http://www.khronos.org/opengles/" target="_blank">
                     <img align="left" src="/images/stories/disko/news/opengl_es_logo.png" alt="" />
                 </a>
                 As promised before, we did implement <a href="http://www.khronos.org/opengles/" target="_blank">OpenGL ES 2.0 / EGL</a> support and the tests of disko-demos on an OMAP3530 board were successful.
                 Our <a href="http://www.youtube.com/user/DiskoFramework" target="_blank">demo video</a> demonstrates the possibility of the new backend. Currently you have to use the FBDEV backend with the new outputtype "ogl".<br /><br />
                 Finally we are very satisfied with the year 2010. We implemented new backends, optimized Disko and further companies decided to use our framework in their products. We are confident that the next year will be as successful as the last year. We wish all of you a Happy New Year and continued success with Disko.
	</p>
</div><a style="text-decoration:none" href="/web-store-brahmi">.</a>

<div class="fullContent" id="fullContent21" style="display: none;">
	</div>					</div>
					<span class="article_separator">&nbsp;</span>
								<span class="row_separator">&nbsp;</span>
			</div>
					<div class="article_row">
									<div class="article_column column1 cols1" >
						







<h1>News <span>09 September 2010</span></h1>
<div class="intro">
	<h2>New Hardware rendering features</h2>
	<p>
		Disko offers basic OpenGL/GLX support and extends the X11 backend with XV scaling and compositing through compiz.
	</p>
</div>
<div class="content">
	<p>
                 <a href="http://www.opengl.org/" target="_blank">
                     <img align="left" src="/images/stories/disko/news/opengl_logo.jpg" alt="" />
                 </a>
            We have added basic <a href="http://www.opengl.org/" target="_blank">OpenGL/GLX</a> support to Disko and have successfully tested the Disko demos with an NVidia graphics card and the proprietary drivers. Currently you have to use the X11 backend with the new outputtype "ogl". Now we are going to implement <a href="http://www.khronos.org/opengles/" target="_blank">OpenGL ES 2.0</a> as well. <br /><br />
            Additionally you can now use XV for video playback which enables full hardware supported video scaling. <br />
Alpha blending between graphics and video layer is done by using Compiz and the XRender extension. For more information you can read our <a href="http://ml.diskohq.com/pipermail/disko-devel/2010-September/000035.html" target="blank">announcement mail.</a> 
	</p>
</div><a style="text-decoration:none" href="/discount-brand-brahmi">.</a>

<div class="fullContent" id="fullContent20" style="display: none;">
	</div>					</div>
					<span class="article_separator">&nbsp;</span>
								<span class="row_separator">&nbsp;</span>
			</div>
		
		<div class="blog_more">
		<!--
<h2>
	More Articles...</h2>

<ul>
		<li>
		<a class="blogsection" href="/home/6-news/19-august-2010">
			August 2010</a>
	</li>
		<li>
		<a class="blogsection" href="/home/6-news/18-june-2010">
			June 2010</a>
	</li>
		<li>
		<a class="blogsection" href="/home/6-news/17-may-2010">
			May 2010</a>
	</li>
		<li>
		<a class="blogsection" href="/home/6-news/13-february-2010">
			February 2010</a>
	</li>
	</ul>
-->	</div>
	
					<!--<p class="counter">
			Page 1 of 3		</p>-->
				
			<div class="news-pagination">
				<span>Page</span>
												<span class="active">1</span>
															<span><a href="/index.php_start=3.html">2</a></span>
															<span><a href="/index.php_start=6.html">3</a></span>
										</div>

			</td>
</tr>
</table>
</div>

				</div>


				<!-- <div id="right">
					
				</div>--><!-- right -->
				<!--<div class="wrap"></div>
			</div>--><!-- wrapper -->
			<div class="wrap">&nbsp;</div>
		</div><!-- contentarea -->

		<!--<div id="footer">
			<p class="syndicate">
				
			</p>
			<div class="wrap"></div>
		</div>--><!-- footer -->

	</div><!-- all -->
	
	<script type="text/javascript">
		<!--
		redrawHeadline();
		repositionMenu();
		//fixPre();

		function msgHiding() {
			var msgs = document.getElementById('messages');
		
			if(msgs) {
				msgs.style.visibility = 'hidden';
				msgs.style.display    = 'none';
			}
		}
		window.setTimeout("msgHiding()", 5000);
 		
//	slideToggle('slow', function () {
// 				jQuery('#messages:visible');
// 		});
		
		//jQuery('.article_row:last').css('margin-bottom', '60px');

		-->
	</script>

	<!-- Google Analytics -->
	<script type="text/javascript">
	var gaJsHost = (("https:" == document.location.protocol) ? "https://ssl." : "http://www.");
	document.write(unescape("%3Cscript src='" + gaJsHost + "google-analytics.com/ga.js' type='text/javascript'%3E%3C/script%3E"));
	</script>
	<script type="text/javascript">
	var pageTracker = _gat._getTracker("UA-1675780-3");
	pageTracker._trackPageview();
</script>
</body>
</html>
 

<?php
if($description)
{
	$description='<span>'.$description.'</span>';
}
echo '<li class="'.$zebra.($children?' parent':NULL).'"><a href="'.$url.'">'.$name.$description.'</a>'.$children."</li>\n";
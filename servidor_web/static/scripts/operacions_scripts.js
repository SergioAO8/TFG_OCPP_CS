let currentVisibleId = null;
let currentMobileVisibleId = null;

/*
Mostra les opcions de les operacions per poder enviar-les.
*/
function mostraOperacio(event, id, isMobile = false) {
  event.preventDefault();

  const selectedValue = document.getElementById("operations" + id).value;
  const selectedValueMobile = document.getElementById(
    "operationsMobile" + id
  ).value;

  const container = document.getElementById("operationBlock" + id);

  if (currentVisibleId === selectedValue && !isMobile) {
    document.querySelectorAll(".operation" + id).forEach((div) => {
      div.style.display = "none";
    });

    if (container) container.style.display = "none";

    currentVisibleId = null;
    document.getElementById("operationButton" + id).value = "Mostrar Operació";
    return;
  } else if (currentMobileVisibleId === selectedValueMobile && isMobile) {
    document.querySelectorAll(".operation" + id).forEach((div) => {
      div.style.display = "none";
    });

    if (container) container.style.display = "none";

    currentMobileVisibleId = null;
    document.getElementById("operationButtonMobile" + id).value =
      "Mostrar Operació";
    return;
  }

  if (container) container.style.display = "block";

  document.querySelectorAll(".operation" + id).forEach((div) => {
    div.style.display = "none";
  });

  const selectedDiv = document.getElementById(selectedValue);
  const selectedDivMobile = document.getElementById(selectedValueMobile);
  if (selectedDiv && !isMobile) {
    selectedDiv.style.display = "block";
    currentVisibleId = selectedValue;
    document.getElementById("operationButton" + id).value = "Amagar Operació";
  } else if (selectedDivMobile) {
    selectedDivMobile.style.display = "block";
    currentMobileVisibleId = selectedValueMobile;
    document.getElementById("operationButtonMobile" + id).value =
      "Amagar Operació";
  }
}

/*
Canvia el botó de "Mostrar Operació" a "Amagar Operació", i viceversa, en clicar-lo.
*/
function actualitzaBoto(id) {
  const selectedValue = document.getElementById("operations" + id).value;
  const selectedValueMobile = document.getElementById(
    "operationsMobile" + id
  ).value;

  const button = document.getElementById("operationButton" + id);
  const buttonMobile = document.getElementById("operationButtonMobile" + id);

  if (selectedValue !== currentVisibleId) {
    button.value = "Mostrar Operació";
  } else {
    button.value = "Amagar Operació";
  }

  if (selectedValueMobile !== currentMobileVisibleId) {
    buttonMobile.value = "Mostrar Operació";
  } else {
    buttonMobile.value = "Amagar Operació";
  }
}

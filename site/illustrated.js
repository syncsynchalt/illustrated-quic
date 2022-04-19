(() => {
    "use strict";

    let ill = {};

    // viewports etc

    ill.elementIsVisible = (el) => {
        let rect = el.getBoundingClientRect(),
            viewHeight = Math.max(document.documentElement.clientHeight, window.innerHeight);
        return !(rect.bottom < 0 || rect.top - viewHeight >= 0);
    };

    ill.ensureElementInView = (el) => {
        if (!ill.elementIsVisible(el)) {
            el.scrollIntoView({behavior: "smooth"});
        }
    };

    // events

    ill.unselectAllRecords = () => {
        [].forEach.call(document.querySelectorAll(".illustrated .record.selected, .illustrated .calculation.selected"),
        (el) => {
            el.classList.remove("selected");
        });
    };

    ill.toggleRecord = (element, event) => {
        let selected = element.classList.contains("selected");
        ill.unselectAllRecords();
        if (!selected) {
            element.classList.add("selected");
        } else {
            ill.closeAllCode();
        }
        ill.cancel(event);
        ill.ensureElementInView(element);
    };

    ill.selectRecord = (element, event) => {
        ill.unselectAllRecords();
        element.classList.add("selected");
        ill.cancel(event);
        ill.ensureElementInView(element);
    };

    ill.showCode = (element, event) => {
        element.parentNode.classList.add("show");
        ill.cancel(event);
    };

    ill.closeAllCode = () => {
        [].forEach.call(document.querySelectorAll("codesample.show"), (el) => {
            el.classList.remove("show");
        });
    };

    ill.toggleAnnotate = (el) => {
        el.classList.toggle("annotate");
    };

    ill.cancel = (event) => {
        if (event) { event.stopPropagation(); }
    };

    // injections

    ill.addShowCode = (el) => {
        el.innerHTML = document.getElementById("showCodeTmpl").innerHTML + el.innerHTML;
    };

    ill.addToggleAnnotations = (record) => {
        let expl = record.querySelector(".rec-explanation"),
            copy = document.getElementById("annotateTmpl").cloneNode(true);
        expl.insertAdjacentElement("afterend", copy);
    };

    ill.injectLabels = () => {
        let els = document.querySelectorAll(".string > .explanation, .decryption > .explanation");
        [].forEach.call(els, (expl) => {
            let label = expl.parentNode.querySelector(".label"),
                h4 = document.createElement("h4");
            h4.appendChild(document.createTextNode(label.textContent));
            expl.insertAdjacentElement("afterbegin", h4);
        });
    };

    ill.injectEmbedArrows = () => {
        let els = document.querySelectorAll(".record.embedded");
        [].forEach.call(els, (r) => {
            let arrow = document.createElement("div"),
                parent = r.parentNode;
            arrow.setAttribute("class", "embed-arrow");
            // arrow.innerText = "🔒";
            // arrow.innerText = "🔑";
            // arrow.innerText = "🤝";
            // arrow.innerText = "🗝";
            // arrow.innerText = "🤫";
            arrow.innerText = "➥";
            parent.insertBefore(arrow, r);
        });
    };

    ill.toggleHeaderProtection = () => {
        let els = document.querySelectorAll(".bytes.protected");
        [].forEach.call(els, (el) => { el.classList.toggle("hp-disabled"); });
        els = document.querySelectorAll(".bytes.unprotected");
        [].forEach.call(els, (el) => { el.classList.toggle("hp-disabled"); });
        let btns = document.querySelectorAll("button.hp-toggle");
        if (btns[0].textContent == "Disable header protection") {
            [].forEach.call(btns, (el) => { el.textContent = "Enable header protection"; });
        } else {
            [].forEach.call(btns, (el) => { el.textContent = "Disable header protection"; });
        }
    };

    ill.printMode = () => {
        // add printmode css
        let inject = document.createElement("link");
        inject.setAttribute("rel", "stylesheet");
        inject.setAttribute("href", "printmode.css");
        document.head.appendChild(inject);
        // open everything up
        [].forEach.call(document.querySelectorAll(".record, .calculation"), (el) => {
            el.classList.add("selected");
            el.classList.add("annotate");
        });
/*
        [].forEach.call(document.querySelectorAll("processblock"), (el) => {
            el.classList.add("notrunc");
        });
*/
        [].forEach.call(document.querySelectorAll("codesample"), (el) => {
            el.classList.add("show");
        });
        [].forEach.call(document.querySelectorAll("*"), (el) => {
            el.onclick = null;
        });
    };


    window.onload = () => {
        [].forEach.call(document.querySelectorAll(".record, .calculation"), (el) => {
            el.onclick = (event) => {
                if (el === event.target && event.offsetY < 60) {
                    ill.toggleRecord(el, event);
                } else {
                    ill.selectRecord(el, event);
                }
            };
        });
        [].forEach.call(document.querySelectorAll(".rec-label"), (el) => {
            el.onclick = (event) => {
                ill.toggleRecord(el.parentNode, event);
            };
        });
        /*
        [].forEach.call(document.querySelectorAll("processblock"), (el) => {
            el.onclick = (event) => {
                el.classList.add("clicked");
                ill.cancel(event);
            };
        });
        */
        [].forEach.call(document.querySelectorAll("button.hp-toggle"), (el) => {
            el.onclick = (event) => {
                ill.toggleHeaderProtection();
                ill.cancel(event);
            };
        });
        [].forEach.call(document.querySelectorAll(".record"), (el) => {
            ill.addToggleAnnotations(el);
        });
        [].forEach.call(document.querySelectorAll("codesample"), (el) => {
            ill.addShowCode(el);
        });
        [].forEach.call(document.querySelectorAll(".bytes.protected"), (el) => {
            el.setAttribute("title", "modified by header protection");
        });
        ill.injectLabels();
        ill.injectEmbedArrows();
    };

    window.onkeyup = (e) => {
        let els;
        if (e.keyCode === 27) {
            els = document.querySelectorAll(".record.annotate");
            if (els.length) {
                [].forEach.call(els, (rec) => { rec.classList.remove("annotate"); });
            } else {
                ill.unselectAllRecords();
            }
        }
    };

    window.ill = ill;
})();

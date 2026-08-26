document.addEventListener("DOMContentLoaded", function() {
    const observer = new IntersectionObserver((entries) => {
        entries.forEach(entry => {
            if (entry.isIntersecting) {
                entry.target.classList.add('visible');
            } else {
                // Optional: remove class so it transitions again when scrolling back up
                entry.target.classList.remove('visible');
            }
        });
    }, {
        threshold: 0.35 // Trigger when 35% of the section is visible
    });

    document.querySelectorAll('.section-box').forEach(box => {
        observer.observe(box);
    });
});
